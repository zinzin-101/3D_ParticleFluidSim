#pragma once
#include "FluidRenderer.h"
#include "FluidContainer.h"
#include "Particle.h"
#include "SpatialHashGrid.h"
#include "Camera.h"
#include <vector>

class FluidSimulation {
private:
	FluidRenderer renderer;
	FluidContainer container;
	SpatialHashGrid spatialHashGrid;
	std::vector<Particle> particles;
	std::vector<float> densities;
	std::vector<glm::vec3> predictedPositions;

	float accumulatedDeltaTime;

	void initSimulation();

	float smoothingKernel(float radius, float distance);
	float smoothingKernelDerivative(float radius, float distance);
	float viscositySmoothingKernel(float radius, float distance);
	float calculateDensity(unsigned int particleIndex);
	glm::vec3 calculatePressureForce(unsigned int particleIndex);
	float calculateSharedPressure(float density1, float density2);
	glm::vec3 calculateViscosityForce(unsigned int particleIndex);
	void updateDensities();

	float densityToPressure(float density);

	void applyGravity(float dt);
	void handleBoundaries();

	void updateParticleDensities(float dt);
	void applyPressureForce(float dt);
	void applyViscosityForce(float dt);
	void updateParticlePositions(float dt);

public:
	unsigned int numOfParticles;
	float particleSpacing;

	glm::vec3 gravitationalForce;
	float particleRadius;
	float particleMass;
	bool pause;
	bool showContainer;

	float smoothingRadius;

	float targetDensity;
	float pressureMultiplier;

	float viscosityMultiplier;

	FluidSimulation();
	void init();
	void update(float dt);
	void render(Camera* camera);

	void reset();

	FluidRenderer* getRenderer();
	FluidContainer* getContainer();
};