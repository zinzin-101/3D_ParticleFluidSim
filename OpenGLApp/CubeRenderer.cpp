#include "CubeRenderer.h"
#include "CubeRendererConfig.h"

using namespace CubeRendererConfig;

CubeRenderer::CubeRenderer(): cubeVAO(0), cubeVBO(0), cubeEBO(0) {}

void CubeRenderer::init() {
	shader.createShader("shaders/PlaneShader.vert", "shaders/PlaneShader.frag");

    float vertices[] = {
        -0.5f, -0.5f,  0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f 
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,

        1, 5, 6,
        6, 2, 1,

        5, 4, 7,
        7, 6, 5,

        4, 0, 3,
        3, 7, 4,

        3, 2, 6,
        6, 7, 3,

        4, 5, 1,
        1, 0, 4
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void CubeRenderer::draw(Camera* camera, float opacity, glm::mat4 model, bool drawAsOutline) {
    shader.use();
    shader.setMat4("model", model);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    shader.setMat3("normalMatrix", normalMatrix);
    shader.setMat4("view", camera->getViewMatrix());
    shader.setMat4("projection", camera->getProjectionMatrix());
    shader.setVec3("color", DEFAULT_CUBE_COLOR);
    shader.setFloat("opacity", drawAsOutline ? 1.0f : opacity);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CubeRenderer::clean() {
    if (cubeVAO != 0) {
        glDeleteVertexArrays(1, &cubeVAO);
    }
    if (cubeVBO != 0) {
        glDeleteBuffers(1, &cubeVBO);
    }
    if (cubeEBO != 0) {
        glDeleteBuffers(1, &cubeEBO);
    }
}