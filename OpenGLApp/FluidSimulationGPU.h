#pragma once
#include "FluidSimulation.h"
#include "ComputeShader.h"

class FluidSimulationGPU : public FluidSimulation {
protected:
	ComputeShader applyForcesShader;
	ComputeShader densityRelaxationShader;
	ComputeShader updatePositionsShader;
	ComputeShader spatialHashGridCountShader;
	ComputeShader spatialHashGridScanShader;
	ComputeShader spatialHashGridSortShader;

	GLuint ssboPositions;
	GLuint ssboVelocities;
	GLuint ssboDensities;
	GLuint ssboNearDensities;
	GLuint ssboPredictedPositions;
	GLuint ssboDeltas;
	GLuint ssboCellStart;
	GLuint ssboCellEntries;
	GLuint ssboCellLocalCounters;

	template<class T>
	void initShaderBuffer(GLuint& ssbo, std::vector<T>* data, unsigned int size);
	
	void bindShaderBuffers();
	void dispatchShader(ComputeShader& shader, unsigned int n);

	virtual void initSimulation() override;
	virtual void updateSimulation(unsigned int n, float dt) override;
	void updateData();

	void createSpatialHashGrid(bool usePredictedPositions);

public:
	FluidSimulationGPU();
	~FluidSimulationGPU();
};

template<class T>
void FluidSimulationGPU::initShaderBuffer(GLuint& ssbo, std::vector<T>* data, unsigned int size) {
	if (ssbo == 0) {
		glGenBuffers(1, &ssbo);
	}

	T* inputData = data == nullptr ? nullptr : data->data();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(T), inputData, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}