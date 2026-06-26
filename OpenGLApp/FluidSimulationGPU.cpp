#include "FluidSimulationGPU.h"
#include "FluidSimulationConfig.h"

FluidSimulationGPU::FluidSimulationGPU():
	ssboPositions(0),
	ssboVelocities(0),
	ssboDensities(0),
	ssboPredictedPositions(0),
	ssboDeltas(0),
	ssboCellStart(0),
	ssboCellEntries(0),
	ssboCellLocalCounters(0)
{}

FluidSimulationGPU::~FluidSimulationGPU() {
	if (ssboPositions != 0){
		glDeleteBuffers(1, &ssboPositions);
	}

	if (ssboVelocities != 0) {
		glDeleteBuffers(1, &ssboVelocities);
	}

	if (ssboDensities != 0) {
		glDeleteBuffers(1, &ssboDensities);
	}

	if (ssboPredictedPositions != 0) {
		glDeleteBuffers(1, &ssboPredictedPositions);
	}

	if (ssboDeltas != 0) {
		glDeleteBuffers(1, &ssboDeltas);
	}

	if (ssboCellStart != 0) {
		glDeleteBuffers(1, &ssboCellStart);
	}

	if (ssboCellEntries != 0) {
		glDeleteBuffers(1, &ssboCellEntries);
	}

	if (ssboCellLocalCounters != 0) {
		glDeleteBuffers(1, &ssboCellLocalCounters);
	}
}

void FluidSimulationGPU::initSimulation() {
	FluidSimulation::initSimulation();

	if (applyForcesShader.ID == 0) {
		applyForcesShader.CreateShader("compute_shaders/simulation_apply_forces.comp");
	}

	if (densityRelaxationShader.ID == 0) {
		densityRelaxationShader.CreateShader("compute_shaders/simulation_density_relaxation.comp");
	}

	if (spatialHashGridCountShader.ID == 0) {
		spatialHashGridCountShader.CreateShader("compute_shaders/spatial_hash_grid_count.comp");
	}

	if (spatialHashGridScanShader.ID == 0) {
		spatialHashGridScanShader.CreateShader("compute_shaders/spatial_hash_grid_scan.comp");
	}

	if (spatialHashGridSortShader.ID == 0) {
		spatialHashGridSortShader.CreateShader("compute_shaders/spatial_hash_grid_sort.comp");
	}

	unsigned int numberOfParticles = positions.size();
	unsigned int tableSize = 2 * numberOfParticles;

	// add padding for glsl vec4
	std::vector<glm::vec4> paddedPositions(numberOfParticles);
	std::vector<glm::vec4> paddedVelocities(numberOfParticles);
	for (int i = 0; i < numberOfParticles; i++) {
		paddedPositions[i] = glm::vec4(positions[i], 1.0f);
		paddedVelocities[i] = glm::vec4(velocities[i], 0.0f);
	}

	// simulation buffers
	initShaderBuffer<glm::vec4>(ssboPositions, &paddedPositions, numberOfParticles);
	initShaderBuffer<glm::vec4>(ssboVelocities, &paddedVelocities, numberOfParticles);
	initShaderBuffer<glm::vec4>(ssboPredictedPositions, nullptr, numberOfParticles);
	initShaderBuffer<glm::vec4>(ssboDeltas, nullptr, numberOfParticles);
	initShaderBuffer<glm::vec2>(ssboDensities, nullptr, numberOfParticles);

	// spatial hash grid buffers
	initShaderBuffer<unsigned int>(ssboCellStart, nullptr, tableSize + 1);
	initShaderBuffer<unsigned int>(ssboCellEntries, nullptr, numberOfParticles);
	initShaderBuffer<unsigned int>(ssboCellLocalCounters, nullptr, tableSize + 1);

	bindShaderBuffers();
}

void FluidSimulationGPU::bindShaderBuffers() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVelocities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboPredictedPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboDensities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboDeltas);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboCellStart);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboCellEntries);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboCellLocalCounters);
}

void FluidSimulationGPU::updateSimulation(unsigned int n, float dt) {
	for (unsigned int i = 0; i < n; i++) {
		GLuint zero = 0;

	}
}