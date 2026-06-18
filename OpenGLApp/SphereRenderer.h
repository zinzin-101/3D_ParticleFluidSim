#pragma once
#include "Camera.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <shader.h>

class SphereRenderer {
private:
	GLuint sphereVAO;
	GLuint sphereVBO;
	GLuint sphereEBO;

	GLuint instanceVBO;

	Shader instancingShader;
	Shader simpleShader;

public:
	std::vector<glm::vec4> instanceData;

	SphereRenderer();
	~SphereRenderer();
	void init();
	void draw(Camera* camera, glm::mat4 model);
	void drawInstance(Camera* camera, float radius, float renderScale, unsigned int instanceCount);
};