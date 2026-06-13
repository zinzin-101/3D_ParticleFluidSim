#include "FluidSimulation.h"

void FluidSimulation::init() {
	renderer.init();
}

void FluidSimulation::update(float dt) {

}

void FluidSimulation::render(Camera* camera) {
	renderer.render(camera);
}