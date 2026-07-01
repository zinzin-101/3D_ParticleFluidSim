#include "SphereRenderer.h"
#include "SphereRendererConfig.h"
#include <numbers>

using namespace SphereRendererConfig;

SphereRenderer::SphereRenderer() : sphereVAO(0), sphereVBO(0), sphereEBO(0), instanceVBO(0) {}

void SphereRenderer::init() {
    instancingShader.CreateShader("shaders/SimpleInstancingShader.vert", "shaders/SimpleInstancingShader.frag");
    simpleShader.CreateShader("shaders/SimpleShader.vert", "shaders/SimpleShader.frag");

    sphereVAO = 0;
    sphereVBO = 0;
    sphereEBO = 0;

    instanceVBO = 0;
    
    instanceData.resize(MAX_INSTANCES);

    const float PI = (float)std::numbers::pi;

    std::vector<float> tempVerts;
    for (int i = 0; i <= SPHERE_RINGS; i++) {
        float phi = PI * (float)i / SPHERE_RINGS;
        for (int j = 0; j <= SPHERE_SEGMENTS; j++) {
            float theta = 2.0f * PI * (float)j / SPHERE_SEGMENTS;
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);

            tempVerts.insert(tempVerts.end(), { x, y, z, x, y, z });
        }
    }


    std::vector<unsigned int> tempIndices;
    for (int i = 0; i < SPHERE_RINGS; i++) {
        for (int j = 0; j < SPHERE_SEGMENTS; j++) {
            unsigned int v0 = i * (SPHERE_SEGMENTS + 1) + j;
            unsigned int v1 = i * (SPHERE_SEGMENTS + 1) + j + 1;
            unsigned int v2 = (i + 1) * (SPHERE_SEGMENTS + 1) + j;
            unsigned int v3 = (i + 1) * (SPHERE_SEGMENTS + 1) + j + 1;

            tempIndices.emplace_back(v0);
            tempIndices.emplace_back(v1);
            tempIndices.emplace_back(v2);

            tempIndices.emplace_back(v1);
            tempIndices.emplace_back(v3);
            tempIndices.emplace_back(v2);
        }
    }

    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);

    // generate VBO
    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        tempVerts.size() * sizeof(float),
        tempVerts.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);                   // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);

    // instance VBO
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * sizeof(glm::vec4), &instanceData[0], GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

    glVertexAttribDivisor(2, 1);

    // generate EBO
    glGenBuffers(1, &sphereEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        tempIndices.size() * sizeof(unsigned int),
        tempIndices.data(),
        GL_STATIC_DRAW
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SphereRenderer::draw(Camera* camera, glm::mat4 model) {
    simpleShader.use();
    simpleShader.setMat4("model", model);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    simpleShader.setMat3("normalMatrix", normalMatrix);
    simpleShader.setMat4("view", camera->getViewMatrix());
    simpleShader.setMat4("projection", camera->getProjectionMatrix());
    simpleShader.setVec3("color", DEFAULT_SPHERE_COLOR);
    simpleShader.setVec3("camPos", camera->transform.position);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, SPHERE_INDICES_COUNT * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SphereRenderer::drawInstance(Camera* camera, float radius, float renderScale, unsigned int instanceCount) {
    if (instanceCount > MAX_INSTANCES) {
        instanceCount = MAX_INSTANCES;
    }

    if (instanceCount == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(glm::vec4), instanceData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    instancingShader.use();
    //glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    glm::mat3 normalMatrix = glm::mat3(1.0f);
    instancingShader.setMat3("normalMatrix", normalMatrix);
    instancingShader.setMat4("view", camera->getViewMatrix());
    instancingShader.setMat4("projection", camera->getProjectionMatrix());
    instancingShader.setVec3("color", DEFAULT_SPHERE_COLOR);
    instancingShader.setVec3("camPos", camera->transform.position);
    instancingShader.setFloat("radius", radius);
    instancingShader.setFloat("renderScale", renderScale);
    instancingShader.setUInt("numOfParticles", instanceCount);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindVertexArray(sphereVAO);
    glDrawElementsInstanced(GL_TRIANGLES, SPHERE_INDICES_COUNT * 6, GL_UNSIGNED_INT, 0, instanceCount);
    glBindVertexArray(0);
}

void SphereRenderer::clean() {
    if (sphereVAO != 0) {
        glDeleteVertexArrays(1, &sphereVAO);
    }

    if (sphereVBO != 0) {
        glDeleteBuffers(1, &sphereVBO);
    }

    if (sphereEBO != 0) {
        glDeleteBuffers(1, &sphereEBO);
    }
}