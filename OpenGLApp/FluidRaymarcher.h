#pragma once
#include "Camera.h"
#include <glad/glad.h>
#include "ComputeShader.h"
#include <shader.h>

class FluidSimulation;

class FluidRaymarcher {
private:
	GLuint quadVAO;
	GLuint quadVBO;
	GLuint quadEBO;

	GLuint densityVolume;

	ComputeShader densityVolumeShader;
	Shader raymarchingShader;

public:
	unsigned int steps;
	float densityMultiplier;
	float airRefractionIndex;
	float fluidRefractionIndex;
	glm::vec3 colorAbsorbtionCoefficient;
	float isoLevel;
	float isoThresholdMultiplier;
	float surfaceSmoothingRadius;

	FluidRaymarcher();
	void init();
	void render(
		FluidSimulation* simulation,
		GLuint cubeMapTexture,
		Camera* camera, 
		float* planesData, 
		float renderScale,
		GLuint positiionsSSBO,
		GLuint densitiesSSBO, 
		GLuint cellStartSSBO, 
		GLuint cellEndSSBO
	);
	void clean();

	void reloadShader();
};