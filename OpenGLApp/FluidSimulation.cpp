#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include "Utility.h"
#include <iostream>

using namespace FluidSimulationConfig;

FluidSimulation::DensityPair::DensityPair() : density(0.0f), nearDensity(0.0f) {}
FluidSimulation::DensityPair::DensityPair(float density, float nearDensity) : density(density), nearDensity(nearDensity) {}

FluidSimulation::FluidSimulation(): 
    spatialHashGrid(DEFAULT_SMOOTHING_RADIUS, DEFAULT_NUMBER_OF_PARTICLES),
    accumulatedDeltaTime(0.0f),
	numOfParticles(DEFAULT_NUMBER_OF_PARTICLES),
	particleSpacing(DEFAULT_PARTICLE_SPACING),
	gravitationalForce(DEFAULT_GRAVITATIONAL_FORCE), 
	particleRadius(DEFAULT_PARTICLE_RADIUS), 
    particleMass(DEFAULT_PARTICLE_MASS),
	pause(true), 
	showContainer(false),
    drawContainerAsOutline(true),
    smoothingRadius(DEFAULT_SMOOTHING_RADIUS),
    targetDensity(DEFAULT_TARGET_DENSITY),
    pressureMultiplier(DEFAULT_PRESSURE_MULTIPLIER),
    nearPressureMultiplier(DEFAULT_NEAR_PRESSURE_MULTIPLIER),
    viscosityMultiplier(DEFAULT_VISCOSITY)
{ }

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

// SpikyPow3 — used for near density (steeper, short-range)
float FluidSimulation::smoothingKernelPow3(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float v = radius - distance;
    float volume = glm::pi<float>() * std::pow(radius, 5.0f) / 10.0f;
    return (v * v * v) / volume;
}

float FluidSimulation::smoothingKernelPow3Derivative(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    float v = radius - distance;
    float scale = -30.0f / (glm::pi<float>() * std::pow(radius, 5.0f));
    return scale * v * v;
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
        if (i == particleIndex) continue;

        float distance = glm::distance(predictedPositions[particleIndex], predictedPositions[i]);
        density += particleMass * smoothingKernelPow2(smoothingRadius, distance);
        nearDensity += particleMass * smoothingKernelPow3(smoothingRadius, distance);
    }

    return DensityPair(density, nearDensity);
}

glm::vec3 FluidSimulation::calculatePressureForce(unsigned int particleIndex) {
    glm::vec3 pressureForce = glm::vec3(0.0f);

    spatialHashGrid.query(predictedPositions[particleIndex], smoothingRadius);
    for (int query = 0; query < spatialHashGrid.getQuerySize(); query++) {
        unsigned int i = (unsigned int)spatialHashGrid.getQueryId(query);
        if (i == particleIndex) continue;
        float distance = glm::distance(predictedPositions[particleIndex], predictedPositions[i]);
        glm::vec3 dir = glm::vec3(0.0f);
        if (distance != 0.0f) {
            dir = (predictedPositions[i] - predictedPositions[particleIndex]) / distance;
        }
        else {
            dir.x = (2.0f * randFloat()) - 1.0f;
            dir.y = (2.0f * randFloat()) - 1.0f;
            dir.z = (2.0f * randFloat()) - 1.0f;
            dir = glm::normalize(dir);
        }
        float gradient = smoothingKernelPow2Derivative(smoothingRadius, distance);
        float density = densities[i];
        float sharedPressure = calculateSharedPressure(density, densities[particleIndex]);
        if (density > 0.0f) {
            pressureForce += sharedPressure * particleMass * gradient * dir / density;
        }

        float nearGradient = smoothingKernelPow3Derivative(smoothingRadius, distance);
        float nearDensity = nearDensities[i];
        float sharedNearPressure = calculateSharedNearPressure(nearDensity, nearDensities[particleIndex]);
        if (nearDensity > 0.0f) {
            pressureForce += sharedNearPressure * particleMass * nearGradient * dir / nearDensity;
        }
    }

    return pressureForce;
}

float FluidSimulation::calculateSharedPressure(float density1, float density2) {
    float p1 = densityToPressure(density1);
    float p2 = densityToPressure(density2);
    return (p1 + p2) / 2.0f;
}

float FluidSimulation::calculateSharedNearPressure(float nearDensity1, float nearDensity2) {
    float p1 = nearDensity1 * nearPressureMultiplier;
    float p2 = nearDensity2 * nearPressureMultiplier;
    return (p1 + p2) / 2.0f;
}

glm::vec3 FluidSimulation::calculateViscosityForce(unsigned int particleIndex) {
    glm::vec3 viscosityForce = glm::vec3(0.0f);
    glm::vec3 position = predictedPositions[particleIndex];

    spatialHashGrid.query(position, smoothingRadius);
    for (int query = 0; query < spatialHashGrid.getQuerySize(); query++) {
        unsigned int i = (unsigned int)spatialHashGrid.getQueryId(query);
        if (i == particleIndex) continue;
        float distance = glm::distance(position, predictedPositions[i]);
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
		//particle.position += particle.velocity * dt;

        predictedPositions[i] = positions[i] + velocities[i] * dt;
	}
}

void FluidSimulation::handleBoundaries() {
	for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
		/*if (!container.IsInside(particle, particleRadius)) {
			container.ResolveCollision(particle, particleRadius);
		}*/

		container.resolveCollision(positions[i], velocities[i], particleRadius);
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

void FluidSimulation::applyPressureForce(float dt) {
    for (unsigned int i = 0; i < (unsigned int)positions.size(); i++) {
        glm::vec3 pressureForce = calculatePressureForce(i);
        if (densities[i] > 0.0f) {
            glm::vec3 pressureAcceleration = pressureForce / densities[i];
            velocities[i] += pressureAcceleration * dt;
            //particles[i].velocity = -pressureAcceleration * dt;
        }
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
        positions[i] += velocities[i] * dt;
        velocities[i] *= DEFAULT_VELOCITY_DAMPING;

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

    int iterations = (int)(accumulatedDeltaTime / FIXED_DT);
    accumulatedDeltaTime -= FIXED_DT * (float)iterations;

    float subStepDeltaTime = FIXED_DT / SIMULATION_STEPS;

    int n = iterations * SIMULATION_STEPS;
    if (n > MAX_SIMULATION_STEPS) {
        n = MAX_SIMULATION_STEPS;
    }

    for (int i = 0; i < n; i++) {
        applyGravity(subStepDeltaTime);
        spatialHashGrid.createHashGrid(predictedPositions);
        updateParticleDensities(subStepDeltaTime);
        applyPressureForce(subStepDeltaTime);
        applyViscosityForce(subStepDeltaTime);
        updateParticlePositions(subStepDeltaTime);
    }

	//applyGravity(dt);

    //updateDensities();

	//handleBoundaries();

	//glm::vec3 pos = particles.at(0).position;
	//glm::vec3 vel = particles.at(0).velocity;
	//std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
	//std::cout << "vel: " << vel.x << " " << vel.y << " " << vel.z << std::endl;
}

void FluidSimulation::reset() {
    clearParticles();
	initSimulation();
    spatialHashGrid.reset(smoothingRadius, (int)positions.size());
    pause = true;
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