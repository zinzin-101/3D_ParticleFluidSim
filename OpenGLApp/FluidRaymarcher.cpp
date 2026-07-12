#include "FluidRaymarcher.h"
#include "FluidRaymarcherConfig.h"
#include "FluidSimulationGPU.h"

using namespace FluidRaymarcherConfig;

FluidRaymarcher::FluidRaymarcher() :
    quadVAO(0),
    quadVBO(0),
    quadEBO(0),
    densityVolume(0),
    steps(DEFAULT_STEPS),
    densityMultiplier(DEFAULT_DENSITY_MULTIPLIER),
    airRefractionIndex(DEFAULT_AIR_REFRACTION_INDEX),
    fluidRefractionIndex(DEFAULT_FLUID_REFRACTION_INDEX),
    colorAbsorbtionCoefficient(DEFAULT_LIGHT_COLOR),
    isoLevel(DEFAULT_ISO_LEVEL),
    isoThresholdMultiplier(DEFAULT_ISO_THRESHOLD_MULTIPLIER),
    surfaceSmoothingRadius(DEFAULT_SURFACE_SMOOTHING_RADIUS),
    maxNumOfBounces(DEFAULT_MAX_NUM_OF_BOUNCES)
{}
 
void FluidRaymarcher::init() {
    densityVolumeShader.createShader("compute_shaders/rendering_create_density_texture.comp");
	raymarchingShader.createShader("shaders/raymarching_with_texture.vert", "shaders/raymarching_with_texture.frag");

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

    // quad
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

    // density 3d texture
    glGenTextures(1, &densityVolume);
    glBindTexture(GL_TEXTURE_3D, densityVolume);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16F, DEFAULT_DENSITY_TEXTURE_RESOLUTION.x, DEFAULT_DENSITY_TEXTURE_RESOLUTION.y, DEFAULT_DENSITY_TEXTURE_RESOLUTION.z);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glBindImageTexture(0, densityVolume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R16F); // bind to compute shader
}

void FluidRaymarcher::render(
    FluidSimulation* simulation,
    GLuint cubeMapTexture, 
    Camera* camera, float* planesData, 
    float renderScale, 
    GLuint positionsSSBO, 
    GLuint densitiesSSBO, 
    GLuint cellStartSSBO, 
    GLuint cellEndSSBO,
    GLuint obstacleDepthTexture
) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densitiesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, cellStartSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, cellEndSSBO);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    densityVolumeShader.use();
    glm::mat3 rotation = simulation->getContainer()->getCurrentBasis();
    glm::vec3 halfsize = simulation->getContainer()->getCurrentScale();
    glm::vec3 origin = simulation->getContainer()->getCurrentPosition() - (rotation * halfsize);
    glm::vec3 fullSize = halfsize * 2.0f;
    densityVolumeShader.setVec3("containerOrigin", origin);
    densityVolumeShader.setMat3("containerRotation", rotation);
    densityVolumeShader.setVec3("containerSize", fullSize);
    densityVolumeShader.setIVec3("containerResolution", DEFAULT_DENSITY_TEXTURE_RESOLUTION);
    densityVolumeShader.setFloat("surfaceSmoothingRadius", surfaceSmoothingRadius);
    densityVolumeShader.setFloat("particleMass", simulation->particleMass);
    densityVolumeShader.setFloat("spacing", simulation->smoothingRadius);
    densityVolumeShader.setUInt("tableSize", 2 * simulation->numOfParticles);
    glm::ivec3 groups = (DEFAULT_DENSITY_TEXTURE_RESOLUTION + glm::ivec3(DENSITY_CALCULATION_GROUP_SIZE - 1)) / glm::ivec3(DENSITY_CALCULATION_GROUP_SIZE);
    glDispatchCompute(groups.x, groups.y, groups.z);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	raymarchingShader.use();
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, densityVolume);
    raymarchingShader.setInt("densityVolume", 2);
    raymarchingShader.setVec3("volumeOrigin", origin);
    raymarchingShader.setMat3("volumeRotation", rotation);
    raymarchingShader.setVec3("volumeSize", fullSize);

    raymarchingShader.setVec3("camPos", camera->transform.position);
    raymarchingShader.setMat4("invViewProj", glm::inverse(camera->getProjectionMatrix() * camera->getViewMatrix()));
    glUniform4fv(glGetUniformLocation(raymarchingShader.ID, "planes"), 6, planesData);
    raymarchingShader.setFloat("renderScale", renderScale);
    raymarchingShader.setUInt("stepCount", steps);
    raymarchingShader.setFloat("densityMultiplier", densityMultiplier);
    raymarchingShader.setFloat("particleMass", simulation->particleMass);
    raymarchingShader.setFloat("spacing", simulation->smoothingRadius);
    raymarchingShader.setUInt("tableSize", 2 * simulation->numOfParticles);

    raymarchingShader.setVec3("absorbtionCoefficient", colorAbsorbtionCoefficient);
    raymarchingShader.setFloat("refractionIndexAir", airRefractionIndex);
    raymarchingShader.setFloat("refractionIndexFluid", fluidRefractionIndex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
    raymarchingShader.setInt("skybox", 1);
    raymarchingShader.setFloat("isoLevel", isoLevel);
    raymarchingShader.setFloat("isoThresholdMultiplier", isoThresholdMultiplier);
    raymarchingShader.setFloat("surfaceSmoothingRadius", surfaceSmoothingRadius);
    raymarchingShader.setUInt("maxNumOfBounces", maxNumOfBounces);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, obstacleDepthTexture);
    raymarchingShader.setInt("sceneDepth", 3);
    raymarchingShader.setFloat("nearPlane", camera->nearPlane);
    raymarchingShader.setFloat("farPlane", camera->farPlane);

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

    if (densityVolume != 0) {
        glDeleteTextures(1, &densityVolume);
    }
}

void FluidRaymarcher::reloadShader() {
    densityVolumeShader.reload();
    raymarchingShader.reload();
}