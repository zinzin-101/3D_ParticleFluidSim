#include "FluidContainer.h"
#include "FluidSimulationConfig.h"
#include "PlaneRendererConfig.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace FluidSimulationConfig;
using namespace PlaneRendererConfig;

FluidContainer::FluidContainer(): planeOpacity(DEFAULT_PLANE_OPACITY) {
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

	currentScale = glm::vec3(halfDefaultContainerLength);
	currentPosition = DEFAULT_CUBE_CONTAINER_ORIGIN;
}

bool FluidContainer::isInside(const Particle& particle, float radius) {
	for (int i = 0; i < 6; i++) {
		float h = glm::dot(glm::vec4(particle.getPosition(), 1.0f), planes[i]);
		if (h - radius >= 0.0f) return false;
	}

	return true;
}

void FluidContainer::resolveCollision(Particle& particle, float radius) {
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
	if (std::abs(scaling.x) > 0.01f && scaling.x + currentScale.x > 0.1f) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.x;
			glm::vec3 normal(planes[i].x, planes[i].y, planes[i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[i].w -= moveDist * lengthSquared;
		}

		currentScale.x += scaling.x;
	}

	if (std::abs(scaling.y) > 0.01f && scaling.y + currentScale.y > 0.1f) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.y;
			glm::vec3 normal(planes[2 + i].x, planes[2 + i].y, planes[2 + i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[2 + i].w -= moveDist * lengthSquared;
		}

		currentScale.y += scaling.y;
	}

	if (std::abs(scaling.z) > 0.01f && scaling.z + currentScale.z > 0.1f) {
		for (int i = 0; i < 2; i++) {
			float moveDist = scaling.z;
			glm::vec3 normal(planes[4 + i].x, planes[4 + i].y, planes[4 + i].z);
			float lengthSquared = glm::dot(normal, normal);
			planes[4 + i].w -= moveDist * lengthSquared;
		}

		currentScale.z += scaling.z;
	}
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
			rotatedPlane.x /= length;
			rotatedPlane.y /= length;
			rotatedPlane.z /= length;
			rotatedPlane.w /= length;
		}

		planes[i] = rotatedPlane;
	}
}

void FluidContainer::visualize(Camera* camera, float renderScale) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	for (int i = 0; i < 6; i++) {
		glm::vec3 normal(planes[i].x, planes[i].y, planes[i].z);

		float signedDist = glm::dot(normal, currentPosition) + planes[i].w;
		glm::vec3 faceCenter = currentPosition - signedDist * normal;

		glm::vec3 from(0.0f, 0.0f, 1.0f);
		glm::vec3 to = normal;

		glm::mat4 rotMat(1.0f);
		float cosA = glm::dot(from, to);
		if (cosA < -0.99f) {
			glm::vec3 perp = (std::abs(from.x) < 0.9f)
				? glm::normalize(glm::cross(from, glm::vec3(1.0f, 0.0f, 0.0f)))
				: glm::normalize(glm::cross(from, glm::vec3(0.0f, 1.0f, 0.0f)));
			rotMat = glm::mat4(glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), perp)));
		}
		else if (cosA < 0.99f) {
			glm::vec3 rotAxis = glm::normalize(glm::cross(from, to));
			float angle = std::acos(cosA);
			rotMat = glm::mat4(glm::mat3(glm::rotate(glm::mat4(1.0f), angle, rotAxis)));
		}

		glm::vec3 faceScale(1.0f);
		if (i <= 1) {
			faceScale = glm::vec3(currentScale.z * 2.0f, currentScale.y * 2.0f, 1.0f);
		}
		else if (i <= 3) {
			faceScale = glm::vec3(currentScale.x * 2.0f, currentScale.z * 2.0f, 1.0f);
		}
		else {
			faceScale = glm::vec3(currentScale.x * 2.0f, currentScale.y * 2.0f, 1.0f);
		}

		glm::mat4 model(1.0f);
		model = glm::scale(model, glm::vec3(renderScale));
		model = glm::translate(model, faceCenter) * rotMat;
		model = glm::scale(model, faceScale);

		planeVisualizer.draw(camera, planeOpacity, model);
	}

	//glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_TRUE);
}

glm::vec3 FluidContainer::getCurrentPosition() const {
	return currentPosition;
}

glm::vec3 FluidContainer::getCurrentScale() const {
	return currentScale;
}