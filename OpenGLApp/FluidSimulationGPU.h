#pragma once
#include "FluidSimulation.h"
#include "ComputeShader.h"

class FluidSimulationGPU : public FluidSimulation {
protected:
	ComputeShader applyForcesShader;
	ComputeShader densityShader;
	ComputeShader densityRelaxationShader;
	ComputeShader updateDeltasShader;
	ComputeShader updatePositionsShader;
	ComputeShader spatialHashGridKeysShader;
	ComputeShader spatialHashGridReorderShader;
	ComputeShader spatialHashGridBoundsShader;

	GLuint ssboPositions;
	GLuint ssboVelocities;
	GLuint ssboDensities;
	GLuint ssboNearDensities;
	GLuint ssboPredictedPositions;
	GLuint ssboDeltas;

	// parallel spatial hash grid construction
	GLuint ssboGridParticleKeys;
	GLuint ssboGridParticleValues;
	GLuint ssboCellStart;
	GLuint ssboCellEnd;
	GLuint ssboSortedPositions;
	GLuint ssboSortedPredictedPositions;
	GLuint ssboSortedVelocities;
	GLuint ssboSortedDensities;
	GLuint ssboSortedNearDensities;

	// gpu radix sort
	ComputeShader radixCountShader;
	ComputeShader radixScanShader;
	ComputeShader radixScatterShader;

	GLuint ssboRadixTempKeys;
	GLuint ssboRadixTempValues;
	GLuint ssboRadixHistograms; // for block counts

	template<class T>
	void initShaderBuffer(GLuint& ssbo, std::vector<T>* data, unsigned int size);
	
	void bindShaderBuffers();
	void dispatchCurrentShader(unsigned int n);

	virtual void initSimulation() override;
	virtual void updateSimulation(unsigned int n, float dt) override;
	void updateData();

	void createSpatialHashGrid(unsigned int tableSize, bool usePredictedPositions);
	void runGPURadixSort(unsigned int numberOfParticles);

	void applyForces(float dt);
	void computeDensities(float dt);
	void updateDeltas();
	void updatePositions(float dt);

public:
	FluidSimulationGPU();
	~FluidSimulationGPU();

	virtual void reset() override;
};

template<class T>
void FluidSimulationGPU::initShaderBuffer(GLuint& ssbo, std::vector<T>* data, unsigned int size) {
	if (ssbo == 0) {
		glGenBuffers(1, &ssbo);
	}

	T* inputData = data == nullptr ? nullptr : data->data();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

	if (inputData != nullptr) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(T), inputData, GL_DYNAMIC_DRAW);
	}
	else {
		std::vector<T> defaultValues(size);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(T), defaultValues.data(), GL_DYNAMIC_DRAW);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}