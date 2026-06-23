#include "FluidRenderer.h"
#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include "SphereRendererConfig.h"

using namespace FluidSimulationConfig;
using namespace SphereRendererConfig;

FluidRenderer::FluidRenderer(): 
    renderScale(DEFAULT_RENDER_SCALE),
    showContainer(true),
    drawContainerAsOutline(true)
{
    camera.farPlane = DEFAULT_RENDER_DISTANCE;
}

void FluidRenderer::init() {
    sphereRenderer.init();
}

void FluidRenderer::update() {
    camera.update();
}

void FluidRenderer::render(FluidSimulation* simulation, bool useInstancing) {
    const std::vector<glm::vec3>& positions = simulation->getPositions(); 
    const std::vector<glm::vec3>& velocities = simulation->getVelocities();
    float radius = simulation->particleRadius;

    if (!useInstancing) {
        for (const glm::vec3& position : positions) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, position * renderScale);
            model = glm::scale(model, glm::vec3(renderScale * radius));
            sphereRenderer.draw(&camera, model);
        }

    }
    else {
        unsigned int idx = 0;
        for (const glm::vec3& position : positions) {
            sphereRenderer.instanceData[idx] = glm::vec4(position, glm::length(velocities[idx]));
            idx++;
            if (idx >= MAX_INSTANCES) break;
        }
        sphereRenderer.drawInstance(&camera, radius, renderScale, idx);
    }

    if (showContainer) {
        FluidContainer* container = simulation->getContainer();
        container->visualize(&camera, renderScale, drawContainerAsOutline);
    }
}

void FluidRenderer::setRenderDistance(float distance) {
    camera.farPlane = distance;
}

Camera* FluidRenderer::getCamera() {
    return &camera;
}