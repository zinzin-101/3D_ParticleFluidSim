#pragma once
#include <glm/glm.hpp>
#include "Particle.h"

class FluidContainer {
private:
	glm::vec4 planes[6];
	glm::vec3 currentScale;
	glm::vec3 currentPosition;

	glm::vec3 getClosestPointOnPlane(const glm::vec3& position, const glm::vec4& plane);

public:
	FluidContainer();
	bool IsInside(const Particle& particle, float radius);
	void ResolveCollision(Particle& particle, float radius);
	void translates(glm::vec3 translation);
	void scales(glm::vec3 scaling);
	void rotates(float degrees, glm::vec3 axis);
};