#include "FluidContainer.h"
#include "FluidSimulationConfig.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace FluidSimulationConfig;

FluidContainer::FluidContainer() {
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

	currentScale = glm::vec3(halfDefaultContainerLength);
	currentPosition = DEFAULT_CUBE_CONTAINER_ORIGIN;
}

glm::vec3 FluidContainer::getClosestPointOnPlane(const glm::vec3& position, const glm::vec4& plane) {
	glm::vec3 normal(plane.x, plane.y, plane.z);
	float h = glm::dot(glm::vec4(position, 1.0f), plane);
	glm::vec3 closestPoint = position - h * normal;

	return closestPoint;
}

bool FluidContainer::IsInside(const Particle& particle, float radius) {
	for (int i = 0; i < 6; i++) {
		float h = glm::dot(glm::vec4(particle.getPosition(), 1.0f), planes[i]);
		if (h - radius >= 0.0f) return false;
	}

	return true;
}

void FluidContainer::ResolveCollision(Particle& particle, float radius) {
	glm::vec3 pos = particle.getPosition();
	glm::vec3 vel = particle.getVelocity();

	for (int i = 0; i < 6; i++) {
		glm::vec3 normal(planes[i].x, planes[i].y, planes[i].z);
		float h = glm::dot(glm::vec4(pos, 1.0f), planes[i]);
		if (h + radius >= 0.0f) {
			glm::vec3 closestPoint = getClosestPointOnPlane(pos, planes[i]);
			pos = closestPoint - normal * radius;

			particle.position = pos;
			particle.velocity = glm::vec3(0.0f);
		}
	}
}

void FluidContainer::translates(glm::vec3 translation) {
	for (int i = 0; i < 6; i++) {
		glm::vec3 normal(planes[i].x, planes[i].y, planes[i].z);
		planes[i].w -= glm::dot(normal, translation);
	}

	currentPosition += translation;
}

void FluidContainer::scales(glm::vec3 scaling) {
	if (std::abs(scaling.x) > 0.01f) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.x;
			glm::vec3 normal(planes[i].x, planes[i].y, planes[i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[i].w -= moveDist * lengthSquared;
		}
	}

	if (std::abs(scaling.y) > 0.01f) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.y;
			glm::vec3 normal(planes[2 + i].x, planes[2 + i].y, planes[2 + i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[2 * i].w -= moveDist * lengthSquared;
		}
	}

	if (std::abs(scaling.z) > 0.01f) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.z;
			glm::vec3 normal(planes[3 + i].x, planes[3 + i].y, planes[3 + i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[3 * i].w -= moveDist * lengthSquared;
		}
	}

	currentScale += scaling;
}

void FluidContainer::rotates(float degrees, glm::vec3 axis) {
	float angle = glm::radians(degrees);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
	glm::mat4 invTranspose = glm::transpose(glm::inverse(rotationMatrix));

	for (int i = 0; i < 6; i++) {
		glm::vec3 oldNormal(planes[i].x, planes[i].y, planes[i].z);
		planes[i].w -= glm::dot(oldNormal, currentPosition);

		glm::vec4 rotatedPlane = invTranspose * planes[i];

		glm::vec3 newNormal(rotatedPlane.x, rotatedPlane.y, rotatedPlane.z);
		rotatedPlane.w += glm::dot(newNormal, currentPosition);

		float length = glm::length(newNormal);
		if (length > 0.0f) {
			rotatedPlane = glm::normalize(rotatedPlane);
		}

		planes[i] = rotatedPlane;
	}
}