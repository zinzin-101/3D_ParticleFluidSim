#include "FluidRenderer.h"
#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include "FluidEngine.h"
#include "SphereRendererConfig.h"
#include "PlaneRendererConfig.h"
#include <stb_image.h>

using namespace FluidSimulationConfig;
using namespace SphereRendererConfig;
using namespace PlaneRendererConfig;

FluidRenderer::FluidRenderer() :
    obstacleDepthFBO(0),
    obstacleDepthTexture(0),
    depthTextureWidth(0),
    depthTextureHeight(0),
    renderScale(DEFAULT_RENDER_SCALE),
    showContainer(true),
    drawContainerAsOutline(true),
    planeOpacity(DEFAULT_PLANE_OPACITY),
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

void FluidRenderer::visualizeCubicContainer(Camera* camera, FluidContainer* container, bool drawAsOutline) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);

    if (drawAsOutline) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    static const int oppositePairs[6] = { 1, 0, 3, 2, 5, 4 };
    static const int adjacentPlanes[6][4] = {
        { 2, 3, 4, 5 },
        { 2, 3, 4, 5 },
        { 0, 1, 4, 5 },
        { 0, 1, 4, 5 },
        { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }
    };

    glm::vec4* planes = container->getPlanesData();
    glm::vec3 currentPosition = container->getCurrentPosition();

    for (int i = 0; i < 6; i++) {
        glm::vec3 ni(planes[i].x, planes[i].y, planes[i].z);
        float di = planes[i].w;

        // find closest point on plane to currentPosition
        float distToPlane = glm::dot(ni, currentPosition) + di;
        glm::vec3 faceCenter = currentPosition - distToPlane * ni;

        int a0 = adjacentPlanes[i][0];
        int a1 = oppositePairs[a0];
        int b0 = -1;
        for (int j : adjacentPlanes[i]) {
            if (j != a0 && j != a1) {
                b0 = j;
                break;
            }
        }

        int b1 = 0;
        if (b0 >= 0) {
            b1 = oppositePairs[b0];
        }

        // solve the 4 actual corners of this face
        // each corner is the intersection of face i + one from {a0,a1} + one from {b0,b1}
        const int cornerCombos[4][2] = { {a0,b0},{a0,b1},{a1,b0},{a1,b1} };

        glm::vec3 corners[4];
        int validCount = 0;
        for (auto& cc : cornerCombos) {
            glm::vec3 nj(planes[cc[0]].x, planes[cc[0]].y, planes[cc[0]].z);
            float dj = planes[cc[0]].w;
            glm::vec3 nk(planes[cc[1]].x, planes[cc[1]].y, planes[cc[1]].z);
            float dk = planes[cc[1]].w;

            glm::mat3 M = glm::transpose(glm::mat3(ni, nj, nk));
            float det = glm::determinant(M);
            if (std::abs(det) < 1e-6f) continue;

            corners[validCount++] = glm::inverse(M) * glm::vec3(-di, -dj, -dk);
        }

        if (validCount < 4) continue;

        // derive tangentU and tangentV from actual face edge directions.
        // corners[0] and corners[1] share the a0 plane  -> their edge is along the b-axis
        // corners[0] and corners[2] share the b0 plane  -> their edge is along the a-axis
        // Use these edges directly as the tangent frame so the bounding box is exact.
        glm::vec3 edgeU = glm::normalize(corners[1] - corners[0]); // along b-pair edge
        glm::vec3 edgeV = glm::normalize(corners[2] - corners[0]); // along a-pair edge

        // distance between opposite edge pairs
        float su = glm::length(corners[1] - corners[0]);
        float sv = glm::length(corners[2] - corners[0]);

        // center of face
        glm::vec3 uvCenter = (corners[0] + corners[1] + corners[2] + corners[3]) * 0.25f;

        glm::mat4 model(1.0f);
        model[0] = glm::vec4(edgeU * su * renderScale, 0.0f);
        model[1] = glm::vec4(edgeV * sv * renderScale, 0.0f);
        model[2] = glm::vec4(ni, 0.0f);
        model[3] = glm::vec4(uvCenter * renderScale, 1.0f);

        planeVisualizer.draw(camera, planeOpacity, model, drawAsOutline);
    }

    //glDisable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDepthMask(GL_TRUE);
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
        visualizeCubicContainer(&camera, simulation->getContainer(), drawContainerAsOutline);
    }
}

void FluidRenderer::cleanup(FluidSimulation* simulation) {
    sphereRenderer.clean();
    planeVisualizer.clean();

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