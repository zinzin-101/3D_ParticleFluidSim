#include "FluidContainer.h"
#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace FluidSimulationConfig;

FluidContainer::FluidContainer(FluidSimulation* simulation) {
	this->simulation = simulation;
	reset();
}

glm::vec3 FluidContainer::getClosestPointOnPlane(const glm::vec3& position, const glm::vec4& plane) {
	glm::vec3 normal(plane.x, plane.y, plane.z);
	float h = glm::dot(glm::vec4(position, 1.0f), plane);
	glm::vec3 closestPoint = position - h * normal;

	return closestPoint;
}

void FluidContainer::reset() {
	static const glm::vec3 CUBE_FACE_DIR[6] = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f)
	};

	float halfDefaultContainerLength = DEFAULT_CUBE_CONTAINER_SIDE_LENGTH / 2.0f;

	for (int i = 0; i < 6; i++) {
		glm::vec3 p = CUBE_FACE_DIR[i] * halfDefaultContainerLength + DEFAULT_CUBE_CONTAINER_ORIGIN;
		float d = -glm::dot(CUBE_FACE_DIR[i], p);
		planes[i] = glm::vec4(CUBE_FACE_DIR[i], d);
	}

	currentRotation = glm::vec3(0.0f);
	currentScale = glm::vec3(halfDefaultContainerLength);
	currentPosition = DEFAULT_CUBE_CONTAINER_ORIGIN;
}

void FluidContainer::reset(unsigned int numOfParticles, float particleSpacing) {
	reset();
	float sideLength = (float)cbrt(numOfParticles) * particleSpacing * 0.25f;
	scales(glm::vec3(sideLength));
}

bool FluidContainer::isInside(const glm::vec3& position, float radius) {
	for (int i = 0; i < 6; i++) {
		float h = glm::dot(glm::vec4(position, 1.0f), planes[i]);
		if (h - radius >= 0.0f) return false;
	}

	return true;
}

void FluidContainer::resolveCollision(glm::vec3& position, glm::vec3& velocity, float radius, bool ignoreVelocity) {
	glm::vec3 pos = position;
	glm::vec3 vel = velocity;

	for (int i = 0; i < 6; i++) {
		glm::vec3 normal(planes[i].x, planes[i].y, planes[i].z);
		float h = glm::dot(glm::vec4(pos, 1.0f), planes[i]);
		if (h + radius >= 0.0f) {
			glm::vec3 closestPoint = getClosestPointOnPlane(pos, planes[i]);
			pos = closestPoint - normal * radius;

			if (!ignoreVelocity) {
				float normalSpeed = glm::dot(normal, vel);
				if (normalSpeed > 0.0f) {
					vel -= 1.0f * normal * normalSpeed;
				}
			}
		}
	}

	position = pos;

	if (!ignoreVelocity) {
		velocity = vel;
	}
}

void FluidContainer::translates(glm::vec3 translation) {
	for (int i = 0; i < 6; i++) {
		glm::vec3 normal(planes[i].x, planes[i].y, planes[i].z);
		planes[i].w -= glm::dot(normal, translation);
	}

	currentPosition += translation;

	// obstacle translations
	unsigned int n = simulation->getObstaclesCount();
	for (unsigned int i = 0; i < n; i++) {
		if (simulation->getObstacleShouldTransformWithContainer(i)) {
			glm::vec3 pos = simulation->getObstaclePosition(i);
			pos += translation;
			simulation->setObstaclePosition(i, pos);
		}
	}
}

void FluidContainer::scales(glm::vec3 scaling) {
	if (std::abs(scaling.x) > 0.01f && scaling.x + currentScale.x > 2.0f * simulation->particleRadius) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.x;
			glm::vec3 normal(planes[i].x, planes[i].y, planes[i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[i].w -= moveDist * lengthSquared;
		}

		currentScale.x += scaling.x;
	}

	if (std::abs(scaling.y) > 0.01f && scaling.y + currentScale.y > 2.0f * simulation->particleRadius) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.y;
			glm::vec3 normal(planes[2 + i].x, planes[2 + i].y, planes[2 + i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[2 + i].w -= moveDist * lengthSquared;
		}

		currentScale.y += scaling.y;
	}

	if (std::abs(scaling.z) > 0.01f && scaling.z + currentScale.z > 2.0f * simulation->particleRadius) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.z;
			glm::vec3 normal(planes[4 + i].x, planes[4 + i].y, planes[4 + i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[4 + i].w -= moveDist * lengthSquared;
		}

		currentScale.z += scaling.z;
	}

	// obstacle scaling
	unsigned int n = simulation->getObstaclesCount();
	for (unsigned int i = 0; i < n; i++) {
		if (simulation->getObstacleShouldTransformWithContainer(i)) {
			glm::vec3 pos = simulation->getObstaclePosition(i);
			pos = ((pos - currentPosition) * ((currentScale) / (currentScale - scaling))) + currentPosition;
			simulation->setObstaclePosition(i, pos);
		}
	}
}

void FluidContainer::rotates(float degrees, glm::vec3 axis) {
	axis = glm::normalize(axis);
	float angle = glm::radians(degrees);
	glm::mat4 identity(1.0f);
	glm::mat4 translateToOrigin = glm::translate(identity, -currentPosition);
	glm::mat4 rotation = glm::rotate(identity, angle, axis);
	glm::mat4 translateBack = glm::translate(identity, currentPosition);
	glm::mat4 modelMatrix = translateBack * rotation * translateToOrigin;
	glm::mat4 planeTransform = glm::transpose(glm::inverse(modelMatrix));

	for (int i = 0; i < 6; i++) {
		glm::vec4 rotatedPlane = planeTransform * planes[i];
		glm::vec3 normal(rotatedPlane.x, rotatedPlane.y, rotatedPlane.z);
		float length = glm::length(normal);
		if (length > 0.0f) {
			rotatedPlane.x /= length;
			rotatedPlane.y /= length;
			rotatedPlane.z /= length;
			rotatedPlane.w /= length;
		}

		planes[i] = rotatedPlane;
	}

	currentRotation.x += degrees * axis.x;
	currentRotation.y += degrees * axis.y;
	currentRotation.z += degrees * axis.z;

	// obstacle rotation
	unsigned int n = simulation->getObstaclesCount();
	for (unsigned int i = 0; i < n; i++) {
		if (simulation->getObstacleShouldTransformWithContainer(i)) {
			glm::vec3 pos = simulation->getObstaclePosition(i);
			pos = modelMatrix * glm::vec4(pos, 1.0f);
			simulation->setObstaclePosition(i, pos);
		}
	}
}

glm::vec3 FluidContainer::getCurrentPosition() const {
	return currentPosition;
}

glm::vec3 FluidContainer::getCurrentScale() const {
	return currentScale;
}

glm::vec3 FluidContainer::getCurrentRotation() const {
	return currentRotation;
}

glm::mat3 FluidContainer::getCurrentBasis() const {
	glm::vec3 right(planes[0].x, planes[0].y, planes[0].z);
	glm::vec3 up(planes[2].x, planes[2].y, planes[2].z);
	glm::vec3 forward(planes[4].x, planes[4].y, planes[4].z);

	right = glm::normalize(right);
	up = glm::normalize(up - right * glm::dot(up, right));
	forward = glm::cross(right, up);

	return glm::mat3(right, up, forward);
}

glm::vec4* FluidContainer::getPlanesData() {
	return planes;
}