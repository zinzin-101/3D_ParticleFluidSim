#pragma once
#include "FluidRenderer.h"
#include "FluidContainer.h"
#include "Particle.h"
#include "SpatialHashGrid.h"
#include "Camera.h"
#include <vector>

class FluidSimulation {
private:
	struct DensityPair {
		float density;
		float nearDensity;
		DensityPair();
		DensityPair(float density, float fnearDensity);
	};

	FluidRenderer renderer;
	FluidContainer container;
	SpatialHashGrid spatialHashGrid;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> velocities;
	std::vector<float> densities;
	std::vector<float> nearDensities;
	std::vector<glm::vec3> predictedPositions;

	float accumulatedDeltaTime;

	unsigned int addParticle(glm::vec3 position, glm::vec3 velocity = glm::vec3(0.0f));
	void clearParticles();

	void initSimulation();

	// kernel functions
	float smoothingKernelPow2(float radius, float distance);
	float smoothingKernelPow2Derivative(float radius, float distance);
	float smoothingKernelPow3(float radius, float distance);
	float smoothingKernelPow3Derivative(float radius, float distance);
	float swmoothingKernelPoly6(float radius, float distance);

	DensityPair calculateDensity(unsigned int particleIndex);
	glm::vec3 calculatePressureForce(unsigned int particleIndex);
	float calculateSharedPressure(float density1, float density2);
	float calculateSharedNearPressure(float nearDensity1, float nearDensity2);
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
	float targetNearDensity;
	float pressureMultiplier;
	float nearPressureMultiplier;

	float viscosityMultiplier;

	FluidSimulation();
	void init();
	void update(float dt);
	void render(Camera* camera);

	void reset();

	FluidRenderer* getRenderer();
	FluidContainer* getContainer();
};