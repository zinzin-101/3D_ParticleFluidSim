#include "Camera.h"
#include "CameraConfig.h"
#include "FluidEngine.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace CameraConfig;

Camera::Camera(): fov(DEFAULT_FOV), nearPlane(DEFAULT_NEAR_PLANE), farPlane(DEFAULT_FAR_PLANE) {
	forward = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	worldUp = up;
	right = glm::vec3(1.0f, 0.0f, 0.0f);

	updateCameraVector();
}

void Camera::setForward(glm::vec3 forward) {
	this->forward = glm::normalize(forward);
	right = glm::normalize(glm::cross(this->forward, worldUp));
	up = glm::normalize(glm::cross(right, this->forward));
}

glm::mat4 Camera::getViewMatrix() const {
	glm::vec3 position = transform.position;
	return glm::lookAt(position, position + forward, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
	glm::vec2 screenDimension = FluidEngine::getInstance()->getScreenDimension();
	return glm::perspective(glm::radians(fov),
		(float)screenDimension.x / (float)screenDimension.y,
		nearPlane, farPlane);
}

glm::vec3 Camera::getFoward() const {
	return forward;
}

glm::vec3 Camera::getRight() const {
	return right;
}

glm::vec3 Camera::getUp() const {
	return up;
}

void Camera::updateCameraVector() {

	glm::vec3 tempForward{};
	// x: pitch, y: yaw, z: roll
	float pitch = transform.eulerRotation.x;
	float yaw = transform.eulerRotation.y - 90.0f;
	float roll = transform.eulerRotation.z;
	tempForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	tempForward.y = sin(glm::radians(pitch));
	tempForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(tempForward);

	right = glm::normalize(glm::cross(forward, worldUp));
	up = glm::normalize(glm::cross(right, forward));
}

void Camera::update() {
	updateCameraVector();
}