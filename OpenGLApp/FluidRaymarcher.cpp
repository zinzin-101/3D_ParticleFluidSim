#include "FluidRaymarcher.h"

void FluidRaymarcher::init() {
	raymarchingShader.CreateShader("shaders/raymarching.vert", "shaders/raymarching.frag");
}

void FluidRaymarcher::render(GLuint densitiesSSBO, GLuint cellStartSSBO, GLuint cellEndSSBO) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densitiesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, cellStartSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, cellEndSSBO);

	raymarchingShader.use();
}