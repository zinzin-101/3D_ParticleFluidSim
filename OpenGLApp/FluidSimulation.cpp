#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include <iostream>

using namespace FluidSimulationConfig;

FluidSimulation::FluidSimulation(): 
	numOfParticles(DEFAULT_NUMBER_OF_PARTICLES),
	particleSpacing(DEFAULT_PARTICLE_SPACING),
	gravitationalForce(DEFAULT_GRAVITATIONAL_FORCE), 
	particleRadius(DEFAULT_PARTICLE_RADIUS), 
	pause(true), 
	showContainer(false) { }

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
}

void FluidSimulation::applyGravity(float dt) {
	for (Particle& particle : particles) {
		particle.velocity += gravitationalForce * dt;
		particle.position += particle.velocity * dt;
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

void FluidSimulation::init() {
	renderer.init();
	initSimulation();
}

void FluidSimulation::update(float dt) {
	if (pause) return;

	applyGravity(dt);
	handleBoundaries();

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
    pause = true;
}

FluidRenderer* FluidSimulation::getRenderer() {
	return &renderer;
}

FluidContainer* FluidSimulation::getContainer() {
	return &container;
}