#pragma once
#include "Camera.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <shader.h>

class SphereRenderer {
private:
	GLuint sphereVAO;
	GLuint sphereVBO;
	GLuint sphereEBO;

	Shader simpleShader;
public:
	SphereRenderer();
	~SphereRenderer();
	void init();
	void draw(Camera* camera, glm::mat4 model);
};