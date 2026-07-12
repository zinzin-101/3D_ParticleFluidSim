#include "CubeMapManager.h"
#include "CubeMapManagerConfig.h"
#include <filesystem>
#include <filesystem.h>

using namespace CubeMapManagerConfig;

CubeMapManager::CubeMapManager(): currentCubeMapIndex(0) {}

void CubeMapManager::init() {
	envMapShader.createShader("shaders/cube_map.vert", "shaders/cube_map.frag");
	envMapShader.use();
	envMapShader.setInt("envMap", 0);

	reloadCubeMaps();
}

void CubeMapManager::render(Camera* camera) {
	if (currentCubeMapIndex >= (unsigned int)cubeMaps.size()) {
		currentCubeMapIndex = 0;
		return;
	}

	envMapShader.use();
	envMapShader.setMat4("projection", camera->getProjectionMatrix());
	envMapShader.setMat4("view", camera->getViewMatrix());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, getCurrentCubeMapTexture());

	glm::mat4 model(1.0f);
	envMapShader.setMat4("model", model);

	cubeMaps.at(currentCubeMapIndex).draw(camera);
}

void CubeMapManager::clean() {
	for (CubeMapRenderer& cubeMap : cubeMaps) {
		cubeMap.clean();
	}
	cubeMaps.clear();
}

void CubeMapManager::setCubeMapIndex(unsigned int index) {
	if (index >= (unsigned int)cubeMaps.size()) return;
	currentCubeMapIndex = index;
}

void CubeMapManager::reloadCubeMaps() {
	clean();

	std::string path = DIRECTORY;
	std::string actualPath = FileSystem::getPath(path);

	std::vector<std::string> paths;

	if (std::filesystem::exists(actualPath) && std::filesystem::is_directory(actualPath)) {
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(actualPath)) {
			if (std::filesystem::is_regular_file(entry.status())) {
				std::string filename = entry.path().filename().string();
				paths.emplace_back(path + filename);
			}
		}
	}

	unsigned int n = (unsigned int)paths.size();
	cubeMaps.resize(n);
	for (int i = n - 1; i >= 0; i--) {
		bool success = cubeMaps[i].init(paths[i], true);
		if (!success) {
			cubeMaps.erase(cubeMaps.begin() + i);
			paths.erase(paths.begin() + i);
		}
	}
}

unsigned int CubeMapManager::getCurrentCubeMapIndex() const {
	return currentCubeMapIndex;
}

std::vector<std::string> CubeMapManager::getCubeMapTextureNames() const {
	std::vector<std::string> names;
	unsigned int startIndex = (unsigned int)DIRECTORY.length();

	for (const CubeMapRenderer& cubeMap : cubeMaps) {
		std::string name = cubeMap.getTexturePath();
		unsigned int length = (unsigned int)(name.size() - DIRECTORY.length());
		name = name.substr(startIndex, length);
		names.emplace_back(name);
	}

	return names;
}

GLuint CubeMapManager::getCurrentCubeMapTexture() const {
	return cubeMaps.at(currentCubeMapIndex).getCubeMapTexture();
}