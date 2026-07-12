#include "FluidRenderer.h"
#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include "FluidEngine.h"
#include "SphereRendererConfig.h"
#include <stb_image.h>

using namespace FluidSimulationConfig;
using namespace SphereRendererConfig;

FluidRenderer::FluidRenderer() :
    obstacleDepthFBO(0),
    obstacleDepthTexture(0),
    renderScale(DEFAULT_RENDER_SCALE),
    showContainer(true),
    drawContainerAsOutline(true),
    showEnvMap(true),
    renderingMode(RenderingMode::BASIC),
    backgroundColor(DEFAULT_BACKGROUND_COLOR)
{
    camera.farPlane = DEFAULT_RENDER_DISTANCE;
    camera.transform.position = glm::vec3(0.0f, 0.0f, 40.0f);
}

void FluidRenderer::init() {
    sphereRenderer.init();
    raymarcher.init();
    cubeMapManager.init();
    //cubeMapRenderer.init("resources/env_map/studio_2k.hdr");

    glm::ivec2 screenDimension = FluidEngine::getInstance()->getScreenDimension();
    createObstacleRenderBuffer((GLsizei)screenDimension.x, (GLsizei)screenDimension.y);
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
        cubeMapManager.getCurrentCubeMapTexture(),
        &camera,
        glm::value_ptr(simulation->getContainer()->getPlanesData()[0]),
        renderScale, 
        simulation->getPositionsSSBO(),
        simulation->getDensitiesSSBO(), 
        simulation->getCellStartSSBO(), 
        simulation->getCellEndSSBO(),
        obstacleDepthTexture
    );
}

void FluidRenderer::createObstacleRenderBuffer(GLsizei width, GLsizei height) {
    depthTextureWidth = width;
    depthTextureHeight = height;

    glGenFramebuffers(1, &obstacleDepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, obstacleDepthFBO);

    glGenTextures(1, &obstacleDepthTexture);
    glBindTexture(GL_TEXTURE_2D, obstacleDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, depthTextureWidth, depthTextureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, obstacleDepthTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Error when creating obstacle depth frame buffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FluidRenderer::renderObstaclesDepth(FluidSimulation* simulation) {
    glBindFramebuffer(GL_FRAMEBUFFER, obstacleDepthFBO);
    glViewport(0, 0, depthTextureWidth, depthTextureHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    renderObstacles(simulation);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FluidRenderer::renderObstacles(FluidSimulation* simulation) {
    unsigned int n = simulation->getObstaclesCount();
    for (unsigned int i = 0; i < n; i++) {
        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(renderScale));
        model = glm::translate(model, simulation->getObstaclePosition(i));
        model = glm::scale(model, glm::vec3(simulation->getObstacleRadius(i)));
        sphereRenderer.draw(&camera, model);
    }
}

void FluidRenderer::render(FluidSimulation* simulation) {
    // depth
    renderObstaclesDepth(simulation);
    glm::ivec2 screenDimension = FluidEngine::getInstance()->getScreenDimension();
    glViewport(0, 0, (GLsizei)screenDimension.x, (GLsizei)screenDimension.y);

    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // cube map
    if (showEnvMap) {
        cubeMapManager.render(&camera);
    }

    // obstacles
    renderObstacles(simulation);

    // fluid
    switch (renderingMode) {
        case RenderingMode::BASIC:
            renderBasic(simulation);
            break;

        case RenderingMode::RAYMARCHING:
            renderRaymarching(simulation);
            break;
    }

    // fluid container
    if (showContainer) {
        FluidContainer* container = simulation->getContainer();
        container->visualize(&camera, renderScale, drawContainerAsOutline);
    }
}

void FluidRenderer::cleanup(FluidSimulation* simulation) {
    sphereRenderer.clean();
    simulation->getContainer()->getPlaneRenderer()->clean();

    raymarcher.clean();

    cubeMapManager.clean();

    if (obstacleDepthFBO != 0) {
        glDeleteBuffers(1, &obstacleDepthFBO);
    }

    if (obstacleDepthTexture != 0) {
        glDeleteTextures(1, &obstacleDepthTexture);
    }
}

void FluidRenderer::setRenderDistance(float distance) {
    camera.farPlane = distance;
}

void FluidRenderer::updateViewport(GLsizei width, GLsizei height) {
    if (width == depthTextureWidth && height == depthTextureHeight) return;

    glDeleteTextures(1, &obstacleDepthTexture);
    glDeleteFramebuffers(1, &obstacleDepthFBO);

    createObstacleRenderBuffer(width, height);
}

Camera* FluidRenderer::getCamera() {
    return &camera;
}

FluidRaymarcher* FluidRenderer::getRaymarcher() {
    return &raymarcher;
}

CubeMapManager* FluidRenderer::getCubeMapManager() {
    return &cubeMapManager;
}