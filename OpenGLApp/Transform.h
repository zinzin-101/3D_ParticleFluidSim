#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class Transform {
public:
	Transform();
	glm::mat4 getGlobalModelMatrix() const;

	glm::vec3 position;

	// Measured in degrees
	glm::vec3 eulerRotation;

	glm::vec3 scale;
};