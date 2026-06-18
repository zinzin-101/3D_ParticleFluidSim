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

	currentRotation = glm::vec3(0.0f);
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

			float normalSpeed = glm::dot(normal, vel);
			if (normalSpeed > 0.0f) {
				vel -= 1.5f * normal * normalSpeed;
			}
		}
	}

	particle.position = pos;
	particle.velocity = vel;
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

	if (std::abs(axis.x) > 0.0f) {
		currentRotation.x += degrees;
	}
	else if (std::abs(axis.y) > 0.0f) {
		currentRotation.y += degrees;
	}
	else if (std::abs(axis.z) > 0.0f) {
		currentRotation.z += degrees;
	}
}

void FluidContainer::visualize(Camera* camera, float renderScale) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	static const int oppositePairs[6] = { 1, 0, 3, 2, 5, 4 };
	static const int adjacentPlanes[6][4] = {
		{ 2, 3, 4, 5 },
		{ 2, 3, 4, 5 },
		{ 0, 1, 4, 5 },
		{ 0, 1, 4, 5 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 }
	};

	for (int i = 0; i < 6; i++) {
		glm::vec3 ni(planes[i].x, planes[i].y, planes[i].z);
		float di = planes[i].w;

		// find closest point on plane to currentPosition
		float distToPlane = glm::dot(ni, currentPosition) + di;
		glm::vec3 faceCenter = currentPosition - distToPlane * ni;

		int a0 = adjacentPlanes[i][0];
		int a1 = oppositePairs[a0];
		int b0 = -1;
		for (int j : adjacentPlanes[i]) {
			if (j != a0 && j != a1) {
				b0 = j;
				break;
			}
		}
		int b1 = oppositePairs[b0];

		// solve the 4 actual corners of this face
		// each corner is the intersection of face i + one from {a0,a1} + one from {b0,b1}
		const int cornerCombos[4][2] = { {a0,b0},{a0,b1},{a1,b0},{a1,b1} };

		glm::vec3 corners[4];
		int validCount = 0;
		for (auto& cc : cornerCombos) {
			glm::vec3 nj(planes[cc[0]].x, planes[cc[0]].y, planes[cc[0]].z);
			float dj = planes[cc[0]].w;
			glm::vec3 nk(planes[cc[1]].x, planes[cc[1]].y, planes[cc[1]].z);
			float dk = planes[cc[1]].w;

			glm::mat3 M = glm::transpose(glm::mat3(ni, nj, nk));
			float det = glm::determinant(M);
			if (std::abs(det) < 1e-6f) continue;

			corners[validCount++] = glm::inverse(M) * glm::vec3(-di, -dj, -dk);
		}

		if (validCount < 4) continue;

		// derive tangentU and tangentV from actual face edge directions.
		// corners[0] and corners[1] share the a0 plane  -> their edge is along the b-axis
		// corners[0] and corners[2] share the b0 plane  -> their edge is along the a-axis
		// Use these edges directly as the tangent frame so the bounding box is exact.
		glm::vec3 edgeU = glm::normalize(corners[1] - corners[0]); // along b-pair edge
		glm::vec3 edgeV = glm::normalize(corners[2] - corners[0]); // along a-pair edge

		// distance between opposite edge pairs
		float su = glm::length(corners[1] - corners[0]);
		float sv = glm::length(corners[2] - corners[0]);

		// center of face
		glm::vec3 uvCenter = (corners[0] + corners[1] + corners[2] + corners[3]) * 0.25f;

		glm::mat4 model(1.0f);
		model[0] = glm::vec4(edgeU * su * renderScale, 0.0f);
		model[1] = glm::vec4(edgeV * sv * renderScale, 0.0f);
		model[2] = glm::vec4(ni, 0.0f);
		model[3] = glm::vec4(uvCenter * renderScale, 1.0f);

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