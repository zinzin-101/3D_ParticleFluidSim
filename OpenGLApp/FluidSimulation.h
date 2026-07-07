#pragma once
#include "FluidRenderer.h"
#include "FluidContainer.h"
#include "SpatialHashGrid.h"
#include "Camera.h"
#include <vector>

class FluidSimulation {
protected:
	struct DensityPair {
		float density;
		float nearDensity;
		DensityPair();
		DensityPair(float density, float fnearDensity);
	};

	FluidContainer container;
	SpatialHashGrid spatialHashGrid;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> velocities;
	std::vector<float> densities;
	std::vector<float> nearDensities;
	std::vector<glm::vec3> predictedPositions;
	std::vector<glm::vec3> deltas;

	float accumulatedDeltaTime;

	unsigned int addParticle(glm::vec3 position, glm::vec3 velocity = glm::vec3(0.0f));
	void clearParticles();

	virtual void initSimulation();
	virtual void updateSimulation(unsigned int n, float dt);

	// kernel functions
	float smoothingKernelPow2(float radius, float distance);
	float smoothingKernelPow2Derivative(float radius, float distance);
	float smoothingKernelPow3(float radius, float distance);
	float smoothingKernelPow3Derivative(float radius, float distance);
	float smoothingKernelPoly6(float radius, float distance);
	float smoothingKernelSpikyDerivative(float radius, float distance);
	float viscosityKernelLaplacian(float radius, float distance);

	DensityPair calculateDensity(unsigned int particleIndex);
	glm::vec3 calculateViscosityForce(unsigned int particleIndex);
	void updateDensities();

	float densityToPressure(float density);

	void applyGravity(float dt);
	void handleBoundaries();

	void updateParticleDensities(float dt);
	void doubleDensityRelaxation(float dt);
	void applyViscosityForce(float dt);
	void updateParticlePositions(float dt);

public:
	unsigned int numOfParticles;
	float particleSpacing;

	glm::vec3 gravitationalForce;
	float particleRadius;
	float particleMass;
	bool pause;
	float timeScale;

	float smoothingRadius;

	float targetDensity;
	float pressureMultiplier;
	float nearPressureMultiplier;

	float viscosityMultiplier;

	FluidSimulation();
	void init();
	void update(float dt);

	virtual void reset();

	FluidContainer* getContainer();
	const std::vector<glm::vec3>& getPositions() const;
	const std::vector<glm::vec3>& getVelocities() const;
	virtual GLuint getPositionsSSBO() const;
	virtual GLuint getVelocitiesSSBO() const;
	virtual GLuint getDensitiesSSBO() const;
	virtual GLuint getCellStartSSBO() const;
	virtual GLuint getCellEndSSBO() const;
};