#include "FluidRenderer.h"
#include <vector>
#include <numbers>

// temp
const unsigned int SPHERE_SEGMENTS = 32;
const unsigned SPHERE_RINGS = 16;
const unsigned int SPHERE_QUADS_COUNTS = (SPHERE_RINGS + 1) * (SPHERE_SEGMENTS + 1);
const unsigned int SPHERE_INDICES_COUNT = SPHERE_RINGS * SPHERE_SEGMENTS;


FluidRenderer::FluidRenderer() :
    sphereVAO(0), sphereVBO(0), sphereEBO(0)
{}

void FluidRenderer::init() {
    simpleShader.CreateShader("shaders/SimpleShader.vert", "shaders/SimpleShader.frag");

	sphereVAO = 0;
	sphereVBO = 0;
	sphereEBO = 0;

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

    // generate EBO
    glGenBuffers(1, &sphereEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        SPHERE_INDICES_COUNT * sizeof(unsigned int) * 6,
        tempIndices.data(),
        GL_STATIC_DRAW
    );
}

FluidRenderer::~FluidRenderer() {
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);
	glDeleteBuffers(1, &sphereEBO);
}

void FluidRenderer::render(Camera* camera) {
    simpleShader.use();
    glm::mat4 model(1.0f);
    simpleShader.setMat4("model", model);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    simpleShader.setMat3("normalMatrix", normalMatrix);
    simpleShader.setMat4("view", camera->getViewMatrix());
    simpleShader.setMat4("projection", camera->getProjectionMatrix());
    simpleShader.setVec3("color", glm::vec3(1.0f));
    simpleShader.setVec3("camPos", camera->transform.position);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, SPHERE_INDICES_COUNT * 6, GL_UNSIGNED_INT, 0);
}