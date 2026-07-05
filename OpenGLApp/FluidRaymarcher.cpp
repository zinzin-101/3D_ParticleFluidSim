#include "FluidRaymarcher.h"
#include "FluidRaymarcherConfig.h"
#include "FluidSimulationGPU.h"

using namespace FluidRaymarcherConfig;

FluidRaymarcher::FluidRaymarcher(): quadVAO(0), quadVBO(0), quadEBO(0), steps(DEFAULT_STEPS), densityMultiplier(DEFAULT_DENSITY_MULTIPLIER) {}
 
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

void FluidRaymarcher::render(
    FluidSimulation* simulation, 
    Camera* camera, float* planesData, 
    float renderScale, 
    GLuint positionsSSBO, 
    GLuint densitiesSSBO, 
    GLuint cellStartSSBO, 
    GLuint cellEndSSBO
) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densitiesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, cellStartSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, cellEndSSBO);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	raymarchingShader.use();
    raymarchingShader.setVec3("camPos", camera->transform.position);
    raymarchingShader.setMat4("invViewProj", glm::inverse(camera->getProjectionMatrix() * camera->getViewMatrix()));
    glUniform4fv(glGetUniformLocation(raymarchingShader.ID, "planes"), 6, planesData);
    raymarchingShader.setFloat("renderScale", renderScale);
    raymarchingShader.setUInt("stepCount", steps);
    raymarchingShader.setFloat("densityMultiplier", densityMultiplier);
    raymarchingShader.setFloat("spacing", simulation->smoothingRadius);
    raymarchingShader.setUInt("tableSize", 2 * simulation->numOfParticles);

    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
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