#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include <iostream>

using namespace FluidSimulationConfig;

FluidSimulation::FluidSimulation(): gravitationalForce(DEFAULT_GRAVITATIONAL_FORCE), particleRadius(DEFAULT_PARTICLE_RADIUS), pause(false), showContainer(false) {}

void FluidSimulation::initSimulation() {
	for (int x = -1; x <= 1; x++) {
		for (int y = 5; y <= 7; y++) {
			for (int z = -1; z <= 1; z++) {
				Particle particle;
				particle.position = glm::vec3(x, y, z) + container.getCurrentPosition();
				particles.emplace_back(particle);
			}
		}
	}
}

void FluidSimulation::applyGravity(float dt) {
	for (Particle& particle : particles) {
		particle.velocity += gravitationalForce * dt;
		particle.position += particle.velocity * dt;
	}
}

void FluidSimulation::handleBoundaries() {
	for (Particle& particle : particles) {
		/*if (!container.IsInside(particle, particleRadius)) {
			container.ResolveCollision(particle, particleRadius);
		}*/

		container.resolveCollision(particle, particleRadius);
	}
}

void FluidSimulation::init() {
	renderer.init();
	initSimulation();
}

void FluidSimulation::update(float dt) {
	if (pause) return;

	applyGravity(dt);
	handleBoundaries();

	//glm::vec3 pos = particles.at(0).position;
	//glm::vec3 vel = particles.at(0).velocity;
	//std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
	//std::cout << "vel: " << vel.x << " " << vel.y << " " << vel.z << std::endl;
}

void FluidSimulation::render(Camera* camera) {
	renderer.render(particles, particleRadius, camera);

	if (showContainer) {
		container.visualize(camera, renderer.renderScale);
	}
}

void FluidSimulation::reset() {
	particles.clear();
	initSimulation();
}

FluidRenderer* FluidSimulation::getRenderer() {
	return &renderer;
}

FluidContainer* FluidSimulation::getContainer() {
	return &container;
}