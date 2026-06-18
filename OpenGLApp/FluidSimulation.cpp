#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include "Utility.h"
#include <iostream>

using namespace FluidSimulationConfig;

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
    smoothingRadius(DEFAULT_SMOOTHING_RADIUS),
    targetDensity(DEFAULT_TARGET_DENSITY),
    pressureMultiplier(DEFAULT_PRESSURE_MULTIPLIER),
    viscosityMultiplier(DEFAULT_VISCOSITY)
{ }

void FluidSimulation::initSimulation() {

    glm::vec3 origin = container.getCurrentPosition();
    unsigned int currentNumOfParticles = 0;
    float increment = particleSpacing;

    float currentX = 0.0f;
    float currentY = 0.0f;
    float currentZ = 0.0f;

    if (currentNumOfParticles < numOfParticles) {
        Particle particle;
        particle.position = origin;
        particles.emplace_back(particle);
        currentNumOfParticles++;
    }

    while (currentNumOfParticles < numOfParticles) {

        currentX += increment;
        currentY += increment;
        currentZ += increment;

        for (float yi = -currentY; yi <= currentY; yi += increment) {
            for (float zi = -currentZ; zi <= currentZ; zi += increment) {
                if (currentNumOfParticles >= numOfParticles) break;

                Particle p1;
                p1.position = glm::vec3(currentX, yi, zi) + origin;
                particles.emplace_back(p1);
                currentNumOfParticles++;

                if (currentNumOfParticles < numOfParticles && currentX > 0) {
                    Particle p2;
                    p2.position = glm::vec3(-currentX, yi, zi) + origin;
                    particles.emplace_back(p2);
                    currentNumOfParticles++;
                }
            }
        }

        for (float xi = -currentX + increment; xi <= currentX - increment; xi += increment) {
            for (float zi = -currentZ; zi <= currentZ; zi += increment) {
                if (currentNumOfParticles >= numOfParticles) break;

                Particle p1;
                p1.position = glm::vec3(xi, currentY, zi) + origin;
                particles.emplace_back(p1);
                currentNumOfParticles++;

                if (currentNumOfParticles < numOfParticles && currentY > 0) {
                    Particle p2;
                    p2.position = glm::vec3(xi, -currentY, zi) + origin;
                    particles.emplace_back(p2);
                    currentNumOfParticles++;
                }
            }
        }

        for (float xi = -currentX + increment; xi <= currentX - increment; xi += increment) {
            for (float yi = -currentY + increment; yi <= currentY - increment; yi += increment) {
                if (currentNumOfParticles >= numOfParticles) break;

                Particle p1;
                p1.position = glm::vec3(xi, yi, currentZ) + origin;
                particles.emplace_back(p1);
                currentNumOfParticles++;

                if (currentNumOfParticles < numOfParticles && currentZ > 0) {
                    Particle p2;
                    p2.position = glm::vec3(xi, yi, -currentZ) + origin;
                    particles.emplace_back(p2);
                    currentNumOfParticles++;
                }
            }
        }
    }

    densities.resize(particles.size());
    predictedPositions.resize(particles.size());
}

float FluidSimulation::smoothingKernel(float radius, float distance) {
    if (distance >= radius) return 0;

    //float volume = (glm::pi<float>() * std::pow(radius, 4.0f)) / 6.0f;
    //return (radius - distance) * (radius - distance) / volume;
    float volume = (glm::pi<float>() * (radius * radius * radius * radius * radius)) / 10.0f;
    float diff = radius - distance;

    return (diff * diff) / volume;
}

float FluidSimulation::smoothingKernelDerivative(float radius, float distance) {
    if (distance >= radius) return 0.0f;
    //float scale = 12.0f / (std::pow(radius, 4.0f) * glm::pi<float>());
    //return (distance - radius) * scale;
    float r6 = radius * radius * radius * radius * radius * radius;
    float scale = -45.0f / (glm::pi<float>() * r6);

    float diff = radius - distance;
    return scale * diff * diff;
}

float FluidSimulation::viscositySmoothingKernel(float radius, float distance) {
    return smoothingKernelDerivative(radius, distance);
}

float FluidSimulation::calculateDensity(unsigned int particleIndex) {
    float density = 0.0f;
    
    spatialHashGrid.query(predictedPositions[particleIndex], smoothingRadius);
    for (int query = 0; query < spatialHashGrid.getQuerySize(); query++) {
        unsigned int i = (unsigned int)spatialHashGrid.getQueryId(query);
        if (i == particleIndex) continue;

        float distance = glm::distance(predictedPositions[particleIndex], predictedPositions[i]);
        float influence = smoothingKernel(smoothingRadius, distance);
        density += particleMass * influence;
    }

    return density;
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
        float gradient = smoothingKernelDerivative(smoothingRadius, distance);
        float density = densities[i];
        float sharedPressure = calculateSharedPressure(density, densities[particleIndex]);
        if (density != 0.0f) {
            pressureForce += sharedPressure * particleMass * gradient * dir / density;
        }
    }

    return pressureForce;
}

float FluidSimulation::calculateSharedPressure(float density1, float density2) {
    float p1 = densityToPressure(density1);
    float p2 = densityToPressure(density2);
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
        float influence = viscositySmoothingKernel(smoothingRadius, distance);
        glm::vec3 relativeVelocity = (particles[i].velocity - particles[particleIndex].velocity);
        if (densities[i] > 0.0f) {
            viscosityForce += (relativeVelocity / densities[i]) * influence;
        }
    }

    return viscosityForce * particleMass * viscosityMultiplier;
}

void FluidSimulation::updateDensities() {
    for (unsigned int i = 0; i < (unsigned int)particles.size(); i++) {
        densities[i] = calculateDensity(i);
    }
}

float FluidSimulation::densityToPressure(float density) {
    float densityDifference = density - targetDensity;
    //float pressure = densityDifference * pressureMultiplier;
    float pressure = (std::max)(0.0f, densityDifference) * pressureMultiplier;
    return pressure;
}

void FluidSimulation::applyGravity(float dt) {
    for (unsigned int i = 0; i < (unsigned int)particles.size(); i++) {
		particles[i].velocity += gravitationalForce * dt;
		//particle.position += particle.velocity * dt;

        predictedPositions[i] = particles[i].position + particles[i].velocity * dt;
	}
}

void FluidSimulation::handleBoundaries() {
	for (Particle& particle : particles) {
		/*if (!container.IsInside(particle, particleRadius)) {
			container.ResolveCollision(particle, particleRadius);
		}*/

		container.resolveCollision(particle, particleRadius);
	}
}

void FluidSimulation::updateParticleDensities(float dt) {
    for (unsigned int i = 0; i < (unsigned int)particles.size(); i++) {
        // apply gravity
        //particles[i].velocity += gravitationalForce * dt;

        // update densities
        densities[i] = calculateDensity(i);
    }
}

void FluidSimulation::applyPressureForce(float dt) {
    for (unsigned int i = 0; i < (unsigned int)particles.size(); i++) {
        glm::vec3 pressureForce = calculatePressureForce(i);
        if (densities[i] != 0.0f) {
            glm::vec3 pressureAcceleration = pressureForce / densities[i];
            particles[i].velocity += pressureAcceleration * dt;
            //particles[i].velocity = -pressureAcceleration * dt;
        }
    }
}

void FluidSimulation::applyViscosityForce(float dt) {
    for (unsigned int i = 0; i < (unsigned int)particles.size(); i++) {
        glm::vec3 viscosityForce = calculateViscosityForce(i);
        particles[i].velocity += viscosityForce  * dt;

    }
}

void FluidSimulation::updateParticlePositions(float dt) {
    for (unsigned int i = 0; i < (unsigned int)particles.size(); i++) {
        // update position
        particles[i].position += particles[i].velocity * dt;
        particles[i].velocity *= DEFAULT_VELOCITY_DAMPING;

        // resolve container collision
        container.resolveCollision(particles[i], particleRadius);
    }
}

void FluidSimulation::init() {
	renderer.init();
	initSimulation();
}

void FluidSimulation::update(float dt) {
	if (pause) return;

    accumulatedDeltaTime += dt;
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
        spatialHashGrid.createHashGrid(particles);
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

void FluidSimulation::render(Camera* camera) {
	renderer.render(particles, particleRadius, camera);

	if (showContainer) {
		container.visualize(camera, renderer.renderScale);
	}
}

void FluidSimulation::reset() {
	particles.clear();
	initSimulation();
    spatialHashGrid.reset(smoothingRadius, (int)particles.size());
    pause = true;
}

FluidRenderer* FluidSimulation::getRenderer() {
	return &renderer;
}

FluidContainer* FluidSimulation::getContainer() {
	return &container;
}