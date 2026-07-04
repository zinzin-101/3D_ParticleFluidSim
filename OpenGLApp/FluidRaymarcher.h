#pragma once
#include "Camera.h"
#include <glad/glad.h>
#include <shader.h>

class FluidSimulation;

class FluidRaymarcher {
private:
	GLuint quadVAO;
	GLuint quadVBO;
	GLuint quadEBO;

	Shader raymarchingShader;

public:
	unsigned int steps;
	float densityMultiplier;

	FluidRaymarcher();
	void init();
	void render(
		FluidSimulation* simulation,
		Camera* camera, 
		float* planesData, 
		float renderScale,
		GLuint positiionsSSBO,
		GLuint densitiesSSBO, 
		GLuint cellStartSSBO, 
		GLuint cellEndSSBO
	);

	void clean();
};