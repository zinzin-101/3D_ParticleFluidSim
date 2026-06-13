#pragma once
#include <glad/glad.h>
#include <shader.h>
#include "Camera.h"

class FluidRenderer {
private:
	GLuint sphereVAO;
	GLuint sphereVBO;
	GLuint sphereEBO;

	Shader simpleShader;
	Camera camera;

public:
	FluidRenderer();
	~FluidRenderer();
	void init();
	void render(Camera* camera);
};