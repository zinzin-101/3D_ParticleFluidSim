#include "Transform.h"

Transform::Transform() {
	position = glm::vec3(0.0f);
	eulerRotation = glm::vec3(0.0f);
	scale = glm::vec3(1.0f);
};

glm::mat4 Transform::getGlobalModelMatrix() const {
	glm::mat4 model(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, scale);
	model = glm::rotate(model, glm::radians(eulerRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(eulerRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(eulerRotation.z), glm::vec3(0.0f, 0.0f, -1.0f));

	return model;
}