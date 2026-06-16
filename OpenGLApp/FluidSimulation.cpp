#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"

using namespace FluidSimulationConfig;

FluidSimulation::FluidSimulation(): gravitationalForce(0.0f), particleRadius(DEFAULT_PARTICLE_RADIUS) {}

void FluidSimulation::initSimulation() {
	for (int x = -1; x <= 1; x++) {
		for (int y = 0; y <= 2; y++) {
			for (int z = -1; z <= 1; z++) {
				Particle particle;
				particle.position = glm::vec3(x, y, z);
				particles.emplace_back(particle);
			}
		}
	}
}

void FluidSimulation::init() {
	renderer.init();
	initSimulation();
}

void FluidSimulation::update(float dt) {

}

void FluidSimulation::render(Camera* camera) {
	renderer.render(particles, particleRadius, camera);
}

FluidRenderer* FluidSimulation::getRenderer() {
	return &renderer;
}