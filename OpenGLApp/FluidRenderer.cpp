#include "FluidRenderer.h"
#include "FluidSimulationConfig.h"
#include "SphereRendererConfig.h"

using namespace FluidSimulationConfig;
using namespace SphereRendererConfig;

FluidRenderer::FluidRenderer(): renderScale(DEFAULT_RENDER_SCALE) {
    camera.farPlane = DEFAULT_RENDER_DISTANCE;
}

void FluidRenderer::init() {
    sphereRenderer.init();
}

void FluidRenderer::update() {
    camera.update();
}

void FluidRenderer::render(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& velocities, float radius, bool useInstancing) {
    if (!useInstancing) {
        for (const glm::vec3& position : positions) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, position * renderScale);
            model = glm::scale(model, glm::vec3(renderScale * radius));
            sphereRenderer.draw(&camera, model);
        }

        return;
    }
    
    unsigned int idx = 0;
    for (const glm::vec3& position : positions) {
        sphereRenderer.instanceData[idx] = glm::vec4(position, glm::length(velocities[idx]));
        idx++;
        if (idx >= MAX_INSTANCES) break;
    }
    sphereRenderer.drawInstance(&camera, radius, renderScale, idx);
}

void FluidRenderer::setRenderDistance(float distance) {
    camera.farPlane = distance;
}

Camera* FluidRenderer::getCamera() {
    return &camera;
}