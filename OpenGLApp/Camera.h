#pragma once
#include "Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:

	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	void updateCameraVector();

public:
	Transform transform;
	float fov;
	float nearPlane;
	float farPlane;
	Camera();
	void setForward(glm::vec3 forward);
	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::vec3 getFoward() const;
	glm::vec3 getRight() const;
	glm::vec3 getUp() const;

	void update();
};