#pragma once
#include "Camera.h"
#include <glad/glad.h>
#include <shader.h>

class FluidRaymarcher {
private:
	GLuint quadVAO;
	GLuint quadVBO;
	GLuint quadEBO;

	Shader raymarchingShader;


public:
	unsigned int steps;

	FluidRaymarcher();
	void init();
	void render(Camera* camera, float* planesData, float renderScale, GLuint densitiesSSBO, GLuint cellStartSSBO, GLuint cellEndSSBO);
	void clean();
};