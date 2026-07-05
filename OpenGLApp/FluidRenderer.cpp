#include "FluidRenderer.h"
#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include "SphereRendererConfig.h"
#include <stb_image.h>

using namespace FluidSimulationConfig;
using namespace SphereRendererConfig;

FluidRenderer::FluidRenderer() :
    renderScale(DEFAULT_RENDER_SCALE),
    showContainer(true),
    drawContainerAsOutline(true),
    showEnvMap(true),
    renderingMode(RenderingMode::BASIC)
{
    camera.farPlane = DEFAULT_RENDER_DISTANCE;
}

void FluidRenderer::init() {
    sphereRenderer.init();
    raymarcher.init();
    cubeMapRenderer.init("resources/env_map/skybox_2k.hdr");
}

void FluidRenderer::update() {
    camera.update();
}

void FluidRenderer::renderBasic(FluidSimulation* simulation) {
    const std::vector<glm::vec3>& positions = simulation->getPositions();
    //const std::vector<glm::vec3>& velocities = simulation->getVelocities();
    float radius = simulation->particleRadius;

    //unsigned int idx = 0;
    //for (const glm::vec3& position : positions) {
    //    sphereRenderer.instanceData[idx] = glm::vec4(position, glm::length(velocities[idx]));
    //    idx++;
    //    if (idx >= MAX_INSTANCES) break;
    //}

    unsigned int instanceCount = (unsigned int)positions.size();
    if (instanceCount > MAX_INSTANCES) {
        instanceCount = MAX_INSTANCES;
    }
    sphereRenderer.drawInstance(
        &camera, 
        radius,
        renderScale,
        simulation->getPositionsSSBO(),
        simulation->getVelocitiesSSBO(),
        instanceCount,
        glm::length(simulation->gravitationalForce)
    );

    //sphereRenderer.drawInstance(&camera, radius, renderScale, idx, glm::length(simulation->gravitationalForce));
}

void FluidRenderer::renderRaymarching(FluidSimulation* simulation) {
    raymarcher.render(
        simulation,
        &camera,
        glm::value_ptr(simulation->getContainer()->getPlanesData()[0]),
        renderScale, 
        simulation->getPositionsSSBO(),
        simulation->getDensitiesSSBO(), 
        simulation->getCellStartSSBO(), 
        simulation->getCellEndSSBO()
    );
}

void FluidRenderer::render(FluidSimulation* simulation) {
    if (showEnvMap) {
        cubeMapRenderer.draw(&camera);
    }

    switch (renderingMode) {
        case RenderingMode::BASIC:
            renderBasic(simulation);
            break;

        case RenderingMode::RAYMARCHING:
            renderRaymarching(simulation);
            break;
    }

    if (showContainer) {
        FluidContainer* container = simulation->getContainer();
        container->visualize(&camera, renderScale, drawContainerAsOutline);
    }
}

void FluidRenderer::cleanup(FluidSimulation* simulation) {
    sphereRenderer.clean();
    simulation->getContainer()->getPlaneRenderer()->clean();

    raymarcher.clean();

    cubeMapRenderer.clean();
}

void FluidRenderer::setRenderDistance(float distance) {
    camera.farPlane = distance;
}

Camera* FluidRenderer::getCamera() {
    return &camera;
}

FluidRaymarcher* FluidRenderer::getRaymarcher() {
    return &raymarcher;
}