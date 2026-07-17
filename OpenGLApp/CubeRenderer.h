#pragma once
#include "Camera.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <shader.h>

class CubeRenderer {
private:
	GLuint cubeVAO;
	GLuint cubeVBO;
	GLuint cubeEBO;

	Shader shader;

public:
	CubeRenderer();
	void init();
	void draw(Camera* camera, float opacity, glm::mat4 model, bool drawAsOutline = false);
	void clean();
};