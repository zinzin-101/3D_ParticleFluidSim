#pragma once
#include <glad/glad.h>
#include <shader.h>

class FluidRaymarcher {
private:
	Shader raymarchingShader;

public:
	void init();
	void render(GLuint densitiesSSBO, GLuint cellStartSSBO, GLuint cellEndSSBO);
};