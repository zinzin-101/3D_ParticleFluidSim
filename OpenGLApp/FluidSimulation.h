#pragma once
#include "FluidRenderer.h"
#include "Particle.h"
#include "Camera.h"
#include <vector>

class FluidSimulation {
private:
	FluidRenderer renderer;
	glm::vec3 gravitationalForce;
	std::vector<Particle> particles;

	void initSimulation();

	void applyGravity(float dt);
	void handleBoundaries();

public:
	float particleRadius;

	FluidSimulation();
	void init();
	void update(float dt);
	void render(Camera* camera);

	void reset();

	FluidRenderer* getRenderer();
};