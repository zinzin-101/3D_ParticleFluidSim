#pragma once
#include <glm/glm.hpp>
#include "PlaneRenderer.h"

class FluidSimulation;

class FluidContainer {
private:
	FluidSimulation* simulation;

	glm::vec4 planes[6];
	glm::vec3 currentRotation;
	glm::vec3 currentScale;
	glm::vec3 currentPosition;

	PlaneRenderer planeVisualizer;

	glm::vec3 getClosestPointOnPlane(const glm::vec3& position, const glm::vec4& plane);


public:
	float planeOpacity;

	FluidContainer(FluidSimulation* simulation);
	bool isInside(const glm::vec3& position, float radius);
	void resolveCollision(glm::vec3& position, glm::vec3& velocity, float radius, bool ignoreVelocity = false);
	void translates(glm::vec3 translation);
	void scales(glm::vec3 scaling);
	void rotates(float degrees, glm::vec3 axis);
	void reset();

	void visualize(Camera* camera, float renderScale, bool drawAsOutline = false);

	glm::vec3 getCurrentPosition() const;
	glm::vec3 getCurrentScale() const;
	glm::vec3 getCurrentRotation() const;

	glm::vec4* getPlanesData();
	PlaneRenderer* getPlaneRenderer();
};