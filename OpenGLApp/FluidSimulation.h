#pragma once
#include "FluidRenderer.h"
#include "FluidContainer.h"
#include "Particle.h"
#include "Camera.h"
#include <vector>

class FluidSimulation {
private:
	FluidRenderer renderer;
	FluidContainer container;
	std::vector<Particle> particles;

	void initSimulation();

	void applyGravity(float dt);
	void handleBoundaries();

public:
	glm::vec3 gravitationalForce;
	float particleRadius;
	bool pause;

	FluidSimulation();
	void init();
	void update(float dt);
	void render(Camera* camera);

	void reset();

	FluidRenderer* getRenderer();
};