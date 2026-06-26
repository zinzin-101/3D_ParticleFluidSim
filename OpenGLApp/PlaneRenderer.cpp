#include "PlaneRenderer.h"
#include "PlaneRendererConfig.h"

using namespace PlaneRendererConfig;

PlaneRenderer::PlaneRenderer(): planeVAO(0), planeVBO(0), planeEBO(0), hasInit(false) {}

void PlaneRenderer::init() {
    planeShader.CreateShader("shaders/PlaneShader.vert", "shaders/PlaneShader.frag");

    float vertices[] = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glGenBuffers(1, &planeEBO);

    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    hasInit = true;
}

void PlaneRenderer::draw(Camera* camera, float opacity, glm::mat4 model, bool drawAsOutline) {
    if (!hasInit) init();
    
    planeShader.use();
    planeShader.setMat4("model", model);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    planeShader.setMat3("normalMatrix", normalMatrix);
    planeShader.setMat4("view", camera->getViewMatrix());
    planeShader.setMat4("projection", camera->getProjectionMatrix());
    planeShader.setVec3("color", DEFAULT_PLANE_COLOR);
    planeShader.setFloat("opacity", drawAsOutline ? 1.0f : opacity);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(planeVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void PlaneRenderer::clean() {
    if (planeVAO != 0) {
        glDeleteVertexArrays(1, &planeVAO);
    }

    if (planeVBO != 0) {
        glDeleteBuffers(1, &planeVBO);
    }

    if (planeEBO != 0) {
        glDeleteBuffers(1, &planeEBO);
    }
}