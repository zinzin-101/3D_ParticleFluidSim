#pragma once
#include <glm/glm.hpp>
#include "Particle.h"
#include "PlaneRenderer.h"

class FluidContainer {
private:
	glm::vec4 planes[6];
	glm::vec3 currentRotation;
	glm::vec3 currentScale;
	glm::vec3 currentPosition;

	PlaneRenderer planeVisualizer;

	glm::vec3 getClosestPointOnPlane(const glm::vec3& position, const glm::vec4& plane);


public:
	float planeOpacity;

	FluidContainer();
	bool isInside(const Particle& particle, float radius);
	void resolveCollision(Particle& particle, float radius);
	void translates(glm::vec3 translation);
	void scales(glm::vec3 scaling);
	void rotates(float degrees, glm::vec3 axis);
	void reset();

	void visualize(Camera* camera, float renderScale);

	glm::vec3 getCurrentPosition() const;
	glm::vec3 getCurrentScale() const;
};