#pragma once
#include "Camera.h"
#include <glad/glad.h>
#include <shader.h>

class CubeMapRenderer {
private:
	std::string path;

	GLuint hdrTexture;
	GLuint cubeMapTexture;

	GLuint captureFBO;
	GLuint captureRBO;
	GLuint cubeVAO;
	GLuint cubeVBO;

	Shader equirectangularToCubeMapShader;
	Shader envMapShader;

public:
	CubeMapRenderer();
	bool init(const std::string& texturePath);
	void draw(Camera* camera);
	void clean();

	std::string getTexturePath() const;
	GLuint getCubeMapTexture() const;
};