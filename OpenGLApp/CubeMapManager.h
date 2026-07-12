#pragma once
#include "CubeMapRenderer.h"
#include <vector>

class CubeMapManager {
private:
	std::vector<CubeMapRenderer> cubeMaps;
	unsigned int currentCubeMapIndex;

	Shader envMapShader;

public:
	CubeMapManager();
	void init();
	void render(Camera* camera);
	void clean();

	void setCubeMapIndex(unsigned int index);
	void reloadCubeMaps();

	unsigned int getCurrentCubeMapIndex() const;
	std::vector<std::string> getCubeMapTextureNames() const;
	GLuint getCurrentCubeMapTexture() const;
};