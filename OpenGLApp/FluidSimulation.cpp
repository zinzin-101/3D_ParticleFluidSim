#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include "FluidEngine.h"
#include "Utility.h"
#include <iostream>
#include <exception>

using namespace FluidSimulationConfig;

FluidSimulation::DensityPair::DensityPair() : density(0.0f), nearDensity(0.0f) {}
FluidSimulation::DensityPair::DensityPair(float density, float nearDensity) : density(density), nearDensity(nearDensity) {}

FluidSimulation::FluidSimulation() :
    container(this),
    spatialHashGrid(DEFAULT_SMOOTHING_RADIUS, DEFAULT_NUMBER_OF_PARTICLES),
    accumulatedDeltaTime(0.0f),
    numOfParticles(DEFAULT_NUMBER_OF_PARTICLES),
    particleSpacing(DEFAULT_PARTICLE_SPACING),
    gravitationalForce(DEFAULT_GRAVITATIONAL_FORCE),
    particleRadius(DEFAULT_PARTICLE_RADIUS),
    particleMass(DEFAULT_PARTICLE_MASS),
    pause(true),
    timeScale(1.0f),
    smoothingRadius(DEFAULT_SMOOTHING_RADIUS),
    targetDensity(DEFAULT_TARGET_DENSITY),
    pressureMultiplier(DEFAULT_PRESSURE_MULTIPLIER),
    nearPressureMultiplier(DEFAULT_NEAR_PRESSURE_MULTIPLIER),
    viscosityMultiplier(DEFAULT_VISCOSITY),
    obstaclesCount(0)
{ 
    obstaclePositions.resize(MAX_NUMBER_OF_OBSTACLES);
    obstacleRadiuses.resize(MAX_NUMBER_OF_OBSTACLES);
    obstacleShouldTransformWithContainer.resize(MAX_NUMBER_OF_OBSTACLES);
}

unsigned int FluidSimulation::addParticle(glm::vec3 position, glm::vec3 velocity) {
    unsigned int index = (unsigned int)positions.size();
    positions.emplace_back(position);
    velocities.emplace_back(velocity);
    return index;
}

void FluidSimulation::clearParticles() {
    positions.clear();
    velocities.clear();
    densities.clear();
    nearDensities.clear();
    predictedPositions.clear();
    deltas.clear();
}

void FluidSimulation::initSimulation() {
    glm::vec3 origin = container.getCurrentPosition();
    unsigned int currentNumOfParticles = 0;
    float increment = particleSpacing;

    float currentX = 0.0f;
    float currentY = 0.0f;
    float currentZ = 0.0f;

    if (currentNumOfParticles < numOfParticles) {
        addParticle(origin);
        currentNumOfParticles++;
    }

    while (currentNumOfParticles < numOfParticles) {

        currentX += increment;
        currentY += increment;
        currentZ += increment;

        for (float yi = -currentY; yi <= currentY; yi += increment) {
            for (float zi = -currentZ; zi <= currentZ; zi += increment) {
                if (currentNumOfParticles >= numOfParticles) break;

                glm::vec3 p1 = glm::vec3(currentX, yi, zi) + origin;
                addParticle(p1);
                currentNumOfParticles++;

                if (currentNumOfParticles < numOfParticles && currentX > 0) {
                    glm::vec3 p2 = glm::vec3(-currentX, yi, zi) + origin;
                    addParticle(p2);
                    currentNumOfParticles++;
                }
            }
        }

        for (float xi = -currentX + increment; xi <= currentX - increment; xi += increment) {
            for (float zi = -currentZ; zi <= currentZ; zi += increment) {
                if (currentNumOfParticles >= numOfParticles) break;

                glm::vec3 p1 = glm::vec3(xi, currentY, zi) + origin;
                addParticle(p1);
                currentNumOfParticles++;

                if (currentNumOfParticles < numOfParticles && currentY > 0) {
                    glm::vec3 p2 = glm::vec3(xi, -currentY, zi) + origin;
                    addParticle(p2);
                    currentNumOfParticles++;
                }
            }
        }

        for (float xi = -currentX + increment; xi <= currentX - increment; xi += increment) {
            for (float yi = -currentY + increment; yi <= currentY - increment; yi += increment) {
                if (currentNumOfParticles >= numOfParticles) break;

                glm::vec3 p1= glm::vec3(xi, yi, currentZ) + origin;
                addParticle(p1);
                currentNumOfParticles++;

                if (currentNumOfParticles < numOfParticles && currentZ > 0) {
                    glm::vec3 p2 = glm::vec3(xi, yi, -currentZ) + origin;
                    addParticle(p2);
                    currentNumOfParticles++;
                }
            }
        }
    }

    densities.resize(positions.size());
    nearDensities.resize(positions.size());
    predictedPositions.resize(positions.size());
    deltas.resize(positions.size());
}

void FluidSimulation::updateSimulation(unsigned int n, float dt) {
    for (unsigned int i = 0; i < n; i++) {
        spatialHashGrid.createHashGrid(positions);
        applyViscosityForce(dt);
        applyGravity(dt);

        for (unsigned int itr = 0; itr < RELAXATION_ITERATIONS; itr++) {
            spatialHashGrid.createHashGrid(predictedPositions);
            updateParticleDensities(dt);
            doubleDensityRelaxation(dt);
            //handleBoundaries();
        }
        updateParticlePositions(dt);
    }
}

float FluidSimulation::smoothingKernelPow2(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float v = radius - distance;
    float volume = glm::pi<float>() * std::pow(radius, 4.0f) / 6.0f;
    return (v * v) / volume;
}

float FluidSimulation::smoothingKernelPow2Derivative(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float v = radius - distance;
    float scale = -12.0f / (glm::pi<float>() * std::pow(radius, 4.0f));
    return scale * v;
}

float FluidSimulation::smoothingKernelPow3(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float v = radius - distance;
    //float volume = glm::pi<float>() * std::pow(radius, 5.0f) / 10.0f;
    //return (v * v * v) / volume;
    float scale = 10.0f / (glm::pi<float>() * std::pow(radius, 5.0f));
    return scale * v * v * v;
}

float FluidSimulation::smoothingKernelPow3Derivative(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float v = radius - distance;
    //float scale = -30.0f / (glm::pi<float>() * std::pow(radius, 5.0f));
    //return scale * v * v;
    float scale = -30.0f / (glm::pi<float>() * std::pow(radius, 5.0f));
    return scale * v * v;
}

float FluidSimulation::smoothingKernelPoly6(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float a = radius * radius - distance * distance;
    return 315.0f / (64.0f * glm::pi<float>() * std::pow(radius, 9.0f)) * a * a * a;
}

float FluidSimulation::smoothingKernelSpikyDerivative(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float v = radius - distance;
    return -45.0f / (glm::pi<float>() * std::pow(radius, 6.0f)) * v * v;
}

float FluidSimulation::viscosityKernelLaplacian(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float scale = 45.0f / (glm::pi<float>() * std::pow(radius, 6.0f));
    return scale * (radius - distance);
}

FluidSimulation::DensityPair FluidSimulation::calculateDensity(unsigned int particleIndex) {
    float density = 0.0f;
    float nearDensity = 0.0f;
    
    spatialHashGrid.query(predictedPositions[particleIndex], smoothingRadius);
    for (int query = 0; query < spatialHashGrid.getQuerySize(); query++) {
        unsigned int i = (unsigned int)spatialHashGrid.getQueryId(query);
        //if (i == particleIndex) continue;

        float distance = glm::distance(predictedPositions[particleIndex], predictedPositions[i]);
        //density += particleMass * smoothingKernelPow2(smoothingRadius, distance);
        density += particleMass * smoothingKernelPoly6(smoothingRadius, distance);
        nearDensity += particleMass * smoothingKernelPow3(smoothingRadius, distance);
    }

    return DensityPair(density, nearDensity);
}

glm::vec3 FluidSimulation::calculateViscosityForce(unsigned int particleIndex) {
    glm::vec3 viscosityForce = glm::vec3(0.0f);
    glm::vec3 position = positions[particleIndex];

    spatialHashGrid.query(position, smoothingRadius);
    for (int query = 0; query < spatialHashGrid.getQuerySize(); query++) {
        unsigned int i = (unsigned int)spatialHashGrid.getQueryId(query);
        if (i == particleIndex) continue;
        float distance = glm::distance(position, positions[i]);
        if (distance <= 0.0f) continue;
        float influence = viscosityKernelLaplacian(smoothingRadius, distance);
        glm::vec3 relativeVelocity = (velocities[i] - velocities[particleIndex]);
        
        float denom = densities[i];
        if (denom > 0.0f) {
            viscosityForce += particleMass * (relativeVelocity / denom) * influence;
        }
    }

    return viscosityForce * viscosityMultiplier;
}

void FluidSimulation::updateDensities() {
    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
        DensityPair densityPair = calculateDensity(i);
        densities[i] = densityPair.density;
        nearDensities[i] = densityPair.nearDensity;
    }
}

float FluidSimulation::densityToPressure(float density) {
    float densityDifference = density - targetDensity;
    float pressure = densityDifference * pressureMultiplier;
    //float pressure = (std::max)(0.0f, densityDifference) * pressureMultiplier;
    //float nearPressure = nearDensity * nearPressureMultiplier;
    //return PressurePair(pressure, nearPressure);
    return pressure;
}

void FluidSimulation::applyGravity(float dt) {
    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
        velocities[i] += gravitationalForce * dt;
        //velocities[i] += accumulatedImpulses[i];
		//particle.position += particle.velocity * dt;

        predictedPositions[i] = positions[i] + velocities[i] * dt;

        //container.resolveCollision(predictedPositions[i], velocities[i], particleRadius);
	}
}

void FluidSimulation::handleBoundaries() {
	for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
		/*if (!container.IsInside(particle, particleRadius)) {
			container.ResolveCollision(particle, particleRadius);
		}*/

        //container.resolveCollision(positions[i], velocities[i], particleRadius);
        container.resolveCollision(predictedPositions[i], velocities[i], particleRadius);
	}
}

void FluidSimulation::updateParticleDensities(float dt) {
    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
        // apply gravity
        //particles[i].velocity += gravitationalForce * dt;

        // update densities
        DensityPair densityPair = calculateDensity(i);
        densities[i] = densityPair.density;
        nearDensities[i] = densityPair.nearDensity;
    }
}

//void FluidSimulation::applyPressureForce(float dt) {
//    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
//        glm::vec3 pressureForce = calculatePressureForce(i);
//        if (densities[i] > 0.0f) {
//            glm::vec3 pressureAcceleration = pressureForce / densities[i];
//            velocities[i] += pressureAcceleration * dt;
//            //particles[i].velocity = -pressureAcceleration * dt;
//            predictedPositions[i] = positions[i] + velocities[i] * dt;
//        }
//
//        //velocities[i] += pressureForce * dt;
//    }
//}

void FluidSimulation::doubleDensityRelaxation(float dt) {
    deltas.assign(deltas.size(), glm::vec3(0.0f));

    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
        float pressure = pressureMultiplier * (densities[i] - targetDensity);
        float nearPressure = nearPressureMultiplier * nearDensities[i];

        spatialHashGrid.query(predictedPositions[i], smoothingRadius);
        for (int query = 0; query < spatialHashGrid.getQuerySize(); query++) {
            unsigned int j = (unsigned int)spatialHashGrid.getQueryId(query);
            if (i == j) continue;

            glm::vec3 rij = predictedPositions[j] - predictedPositions[i];
            float r = glm::length(rij);
            if (r <= 1e-6f || r >= smoothingRadius) continue;

            glm::vec3 dir = glm::normalize(rij);
            float q = 1.0f - (r / smoothingRadius);
            glm::vec3 D = dt * dt * (pressure * q + nearPressure * q * q) * dir;
            deltas[i] -= 0.5f * D;
            deltas[j] += 0.5f * D;
        }
    }

    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++)
    {
        predictedPositions[i] += deltas[i];
    }
}

void FluidSimulation::applyViscosityForce(float dt) {
    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
        glm::vec3 viscosityForce = calculateViscosityForce(i);
        velocities[i] += viscosityForce * dt;
    }
}

void FluidSimulation::updateParticlePositions(float dt) {
    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
        // update position
        //positions[i] += velocities[i] * dt;
        //velocities[i] *= DEFAULT_VELOCITY_DAMPING;
        velocities[i] = (predictedPositions[i] - positions[i]) / dt;
        positions[i] = predictedPositions[i];

        // resolve container collision
        container.resolveCollision(positions[i], velocities[i], particleRadius);
    }
}

void FluidSimulation::init() {
	initSimulation();
}

void FluidSimulation::update(float dt) {
	if (pause) return;

    accumulatedDeltaTime += dt;
    accumulatedDeltaTime = (std::min)(accumulatedDeltaTime, FIXED_DT * (float)SIMULATION_STEPS);
    if (accumulatedDeltaTime < FIXED_DT) return;

    unsigned int iterations = (unsigned int)(accumulatedDeltaTime / FIXED_DT);
    accumulatedDeltaTime -= FIXED_DT * (float)iterations;

    float subStepDeltaTime = FIXED_DT / SIMULATION_STEPS;

    unsigned int n = iterations * SIMULATION_STEPS;
    if (n > MAX_SIMULATION_STEPS) {
        n = MAX_SIMULATION_STEPS;
    }

    updateSimulation(n, subStepDeltaTime * timeScale);

    //float avgspeed = 0.0f;
    //float maxspeed = 0.0f;
    //float avgdensity = 0.0f;
    //float maxdensity = 0.0f;
    //float mindensity = 999999999.0f;
    //for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
    //    glm::vec3 vel = velocities[i];
    //    float density = densities[i];

    //    float speed = glm::length(vel);
    //    avgspeed += speed;
    //    maxspeed = (std::max)(maxspeed, speed);

    //    avgdensity += density;
    //    maxdensity = (std::max)(maxdensity, density);
    //    mindensity = (std::min)(mindensity, density);
    //}

    //avgdensity /= (float)velocities.size();
    //std::cout << "avg density: " << avgdensity << std::endl;
    //std::cout << "max density: " << maxdensity << std::endl;
    //std::cout << "min density: " << mindensity << std::endl;

    //avgspeed /= (float)velocities.size();
    //std::cout << "avg speed: " << avgspeed << std::endl;
    //std::cout << "max speed: " << maxspeed << std::endl;
}

void FluidSimulation::reset() {
    clearParticles();
	initSimulation();
    spatialHashGrid.reset(smoothingRadius, (int)positions.size());
    pause = true;
}

void FluidSimulation::addObstacle(glm::vec3 position, float radius) {
    if (obstaclesCount >= MAX_NUMBER_OF_OBSTACLES) return;

    obstaclePositions[obstaclesCount] = position;
    obstacleRadiuses[obstaclesCount] = radius;
    obstacleShouldTransformWithContainer[obstaclesCount] = DEFAULT_SHOULD_OBSTACLE_TRANSFORM_WITH_CONTAINER;

    obstaclesCount++;
}

void FluidSimulation::removeObstacle(unsigned int index) {
    if (index >= obstaclesCount) {
        throw std::runtime_error("obstacle index greater than obstacle count");
    }

    if (obstaclesCount == 0) return;

    obstaclesCount--;
    for (unsigned int i = index; i < obstaclesCount; i++) {
        obstaclePositions[i] = obstaclePositions[i + 1];
        obstacleRadiuses[i] = obstacleRadiuses[i + 1];
        obstacleShouldTransformWithContainer[i] = obstacleShouldTransformWithContainer[i + 1];
    }
}

glm::vec3 FluidSimulation::getObstaclePosition(unsigned int index) const {
    if (index >= obstaclesCount) {
        throw std::runtime_error("obstacle index greater than obstacle count");
    }

    return obstaclePositions.at(index);
}

float FluidSimulation::getObstacleRadius(unsigned int index) const {
    if (index >= obstaclesCount) {
        throw std::runtime_error("obstacle index greater than obstacle count");
    }

    return obstacleRadiuses.at(index);
}

bool FluidSimulation::getObstacleShouldTransformWithContainer(unsigned int index) const {
    if (index >= obstaclesCount) {
        throw std::runtime_error("obstacle index greater than obstacle count");
    }

    return obstacleShouldTransformWithContainer.at(index);
}

const std::vector<glm::vec3>& FluidSimulation::getObstaclePositions() const {
    return obstaclePositions;
}

const std::vector<float>& FluidSimulation::getObstaclesRadiuses() const {
    return obstacleRadiuses;
}

const std::vector<bool>& FluidSimulation::getAllObstacleShouldTransformWithContainer() const {
    return obstacleShouldTransformWithContainer;
}

unsigned int FluidSimulation::getObstaclesCount() const {
    return obstaclesCount;
}

void FluidSimulation::setObstaclePosition(unsigned int index, glm::vec3 position) {
    if (index >= obstaclesCount) {
        throw std::runtime_error("obstacle index greater than obstacle count");
    }

    obstaclePositions[index] = position;
}

void FluidSimulation::setObstacleRadius(unsigned int index, float radius) {
    if (index >= obstaclesCount) {
        throw std::runtime_error("obstacle index greater than obstacle count");
    }

    obstacleRadiuses[index] = radius;
}

void FluidSimulation::setObstacleShouldTransformWithContainer(unsigned int index, bool value) {
    if (index >= obstaclesCount) {
        throw std::runtime_error("obstacle index greater than obstacle count");
    }

    obstacleShouldTransformWithContainer[index] = value;
}

void FluidSimulation::clearObstacles() {
    obstaclesCount = 0;
}

FluidContainer* FluidSimulation::getContainer() {
	return &container;
}

const std::vector<glm::vec3>& FluidSimulation::getPositions() const {
    return positions;
}

const std::vector<glm::vec3>& FluidSimulation::getVelocities() const {
    return velocities;
}

GLuint FluidSimulation::getPositionsSSBO() const {
    return 0;
}

GLuint FluidSimulation::getVelocitiesSSBO() const {
    return 0;
}

GLuint FluidSimulation::getDensitiesSSBO() const {
    return 0;
}

GLuint FluidSimulation::getCellStartSSBO() const {
    return 0;
}

GLuint FluidSimulation::getCellEndSSBO() const {
    return 0;
}