#include "FluidRenderer.h"
#include "FluidSimulationConfig.h"
#include "SphereRendererConfig.h"

using namespace FluidSimulationConfig;
using namespace SphereRendererConfig;

FluidRenderer::FluidRenderer(): renderScale(DEFAULT_RENDER_SCALE) {}

void FluidRenderer::init() {
    sphereRenderer.init();
}

void FluidRenderer::render(const std::vector<Particle>& particles, float radius, Camera* camera, bool useInstancing) {
    if (!useInstancing) {
        for (const Particle& particle : particles) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, particle.getPosition() * renderScale);
            model = glm::scale(model, glm::vec3(renderScale * radius));
            sphereRenderer.draw(camera, model);
        }

        return;
    }
    
    unsigned int idx = 0;
    for (const Particle& particle : particles) {
        sphereRenderer.instanceData[idx] = glm::vec4(particle.getPosition(), glm::length(particle.getVelocity()));
        idx++;
        if (idx >= MAX_INSTANCES) break;
    }
    sphereRenderer.drawInstance(camera, radius, renderScale, idx);
}