#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"

using namespace FluidSimulationConfig;

FluidSimulation::FluidSimulation(): gravitationalForce(DEFAULT_GRAVITATIONAL_FORCE), particleRadius(DEFAULT_PARTICLE_RADIUS) {}

void FluidSimulation::initSimulation() {
	for (int x = -1; x <= 1; x++) {
		for (int y = 5; y <= 7; y++) {
			for (int z = -1; z <= 1; z++) {
				Particle particle;
				particle.position = glm::vec3(x, y, z);
				particles.emplace_back(particle);
			}
		}
	}

	gravitationalForce = DEFAULT_GRAVITATIONAL_FORCE;
}

void FluidSimulation::applyGravity(float dt) {
	for (Particle& particle : particles) {
		particle.velocity += gravitationalForce * dt;
		particle.position += particle.velocity * dt;
	}
}

void FluidSimulation::handleBoundaries() {
	// simple cube/cuboid


}

void FluidSimulation::init() {
	renderer.init();
	initSimulation();
}

void FluidSimulation::update(float dt) {
	applyGravity(dt);
}

void FluidSimulation::render(Camera* camera) {
	renderer.render(particles, particleRadius, camera);
}

void FluidSimulation::reset() {
	particles.clear();
	initSimulation();
}

FluidRenderer* FluidSimulation::getRenderer() {
	return &renderer;
}