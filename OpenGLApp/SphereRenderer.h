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
	Shader instancingSSBOShader;

public:
	std::vector<glm::vec4> instanceData;

	SphereRenderer();
	void init();
	void draw(Camera* camera, glm::mat4 model);
	void drawInstance(Camera* camera, float radius, float renderScale, unsigned int instanceCount, float gravityStrength = 9.81f);
	void drawInstance(
		Camera* camera,
		float radius,
		float renderScale,
		GLuint positionsSSBO, 
		GLuint velocitySSBO, 
		unsigned int instanceCount,
		float gravityStrength = 9.81f
	);
	void clean();
};