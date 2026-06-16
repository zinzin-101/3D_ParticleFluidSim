#include "FluidRenderer.h"
#include "FluidSimulationConfig.h"

using namespace FluidSimulationConfig;

FluidRenderer::FluidRenderer(): renderScale(DEFAULT_RENDER_SCALE) {}

void FluidRenderer::init() {
    sphereRenderer.init();
}

void FluidRenderer::render(const std::vector<Particle>& particles, float radius, Camera* camera) {
    for (const Particle& particle : particles) {
        glm::mat4 model(1.0f);
        model = glm::translate(model, particle.getPosition() * renderScale);
        model = glm::scale(model, glm::vec3(renderScale * radius));
        sphereRenderer.draw(camera, model);
    }
}