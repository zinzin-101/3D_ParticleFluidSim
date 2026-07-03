#include "FluidRaymarcher.h"
#include "FluidRaymarcherConfig.h"

using namespace FluidRaymarcherConfig;

FluidRaymarcher::FluidRaymarcher(): quadVAO(0), quadVBO(0), quadEBO(0), steps(DEFAULT_STEPS) {}
 
void FluidRaymarcher::init() {
	raymarchingShader.CreateShader("shaders/raymarching.vert", "shaders/raymarching.frag");

    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f,    0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,    1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,    1.0f, 1.0f
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void FluidRaymarcher::render(Camera* camera, float* planesData, float renderScale, GLuint densitiesSSBO, GLuint cellStartSSBO, GLuint cellEndSSBO) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densitiesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, cellStartSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, cellEndSSBO);

	raymarchingShader.use();
    raymarchingShader.setVec3("camPos", camera->transform.position);
    raymarchingShader.setMat4("invView", glm::inverse(camera->getViewMatrix()));
    glUniform4fv(glGetUniformLocation(raymarchingShader.ID, "planes"), 6, planesData);
    raymarchingShader.setFloat("renderScale", renderScale);
    raymarchingShader.setUInt("stepCount", steps);

    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void FluidRaymarcher::clean() {
    if (quadVAO != 0) {
        glDeleteVertexArrays(1, &quadVAO);
    }

    if (quadVBO != 0) {
        glDeleteBuffers(1, &quadVBO);
    }

    if (quadEBO != 0) {
        glDeleteBuffers(1, &quadEBO);
    }
}