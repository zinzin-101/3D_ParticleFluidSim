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
    cubeMapRenderer.init("resources/env_map/skybox_2k.hdr");
}

void FluidRenderer::update() {
    camera.update();
}

void FluidRenderer::renderBasic(FluidSimulation* simulation) {
    const std::vector<glm::vec3>& positions = simulation->getPositions();
    const std::vector<glm::vec3>& velocities = simulation->getVelocities();
    float radius = simulation->particleRadius;

    unsigned int idx = 0;
    for (const glm::vec3& position : positions) {
        sphereRenderer.instanceData[idx] = glm::vec4(position, glm::length(velocities[idx]));
        idx++;
        if (idx >= MAX_INSTANCES) break;
    }
    sphereRenderer.drawInstance(&camera, radius, renderScale, idx, glm::length(simulation->gravitationalForce));
}

void FluidRenderer::renderRaymarching(FluidSimulation* simulation) {

}

void FluidRenderer::render(FluidSimulation* simulation) {
    switch (renderingMode) {
        case RenderingMode::BASIC:
            renderBasic(simulation);
            break;

        case RenderingMode::RAYMARCHING:
            renderRaymarching(simulation);
            break;
    }

    if (showEnvMap) {
        cubeMapRenderer.draw(&camera);
    }

    if (showContainer) {
        FluidContainer* container = simulation->getContainer();
        container->visualize(&camera, renderScale, drawContainerAsOutline);
    }
}

void FluidRenderer::cleanup(FluidSimulation* simulation) {
    sphereRenderer.clean();
    simulation->getContainer()->getPlaneRenderer()->clean();

    cubeMapRenderer.clean();
}

void FluidRenderer::setRenderDistance(float distance) {
    camera.farPlane = distance;
}

Camera* FluidRenderer::getCamera() {
    return &camera;
}

GLuint FluidRenderer::loadTexture(char const* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format{};
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}