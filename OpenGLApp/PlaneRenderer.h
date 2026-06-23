#pragma once
#include "Camera.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <shader.h>

class PlaneRenderer {
private:
	GLuint planeVAO;
	GLuint planeVBO;
	GLuint planeEBO;

	Shader planeShader;
	bool hasInit;

	void init();

public:
	PlaneRenderer();
	~PlaneRenderer();
	void draw(Camera* camera, float opacity, glm::mat4 model, bool drawAsOutline = false);
};