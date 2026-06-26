#include "FluidSimulationGPU.h"
#include "FluidSimulationConfig.h"

using namespace FluidSimulationConfig;

FluidSimulationGPU::FluidSimulationGPU():
	ssboPositions(0),
	ssboVelocities(0),
	ssboDensities(0),
	ssboNearDensities(0),
	ssboPredictedPositions(0),
	ssboDeltas(0),
	ssboGridParticleKeys(0),
	ssboGridParticleValues(0),
	ssboCellStart(0),
	ssboCellEnd(0),
	ssboSortedPositions(0),
	ssboSortedVelocities(0),
	ssboRadixTempKeys(0),
	ssboRadixTempValues(0),
	ssboRadixHistograms(0)
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

	if (ssboNearDensities != 0) {
		glDeleteBuffers(1, &ssboNearDensities);
	}

	if (ssboPredictedPositions != 0) {
		glDeleteBuffers(1, &ssboPredictedPositions);
	}

	if (ssboDeltas != 0) {
		glDeleteBuffers(1, &ssboDeltas);
	}

	if (ssboGridParticleKeys != 0) {
		glDeleteBuffers(1, &ssboGridParticleKeys);
	}

	if (ssboGridParticleValues != 0) {
		glDeleteBuffers(1, &ssboGridParticleValues);
	}

	if (ssboCellStart != 0) {
		glDeleteBuffers(1, &ssboCellStart);
	}

	if (ssboCellEnd != 0) {
		glDeleteBuffers(1, &ssboCellEnd);
	}

	if (ssboSortedPositions != 0) {
		glDeleteBuffers(1, &ssboSortedPositions);
	}

	if (ssboSortedVelocities != 0) {
		glDeleteBuffers(1, &ssboSortedVelocities);
	}

	if (ssboRadixTempKeys != 0) {
		glDeleteBuffers(1, &ssboRadixTempKeys);
	}

	if (ssboRadixTempValues != 0) {
		glDeleteBuffers(1, &ssboRadixTempValues);
	}

	if (ssboRadixHistograms != 0) {
		glDeleteBuffers(1, &ssboRadixHistograms);
	}
}

void FluidSimulationGPU::reset() {
	clearParticles();
	initSimulation();
	pause = true;
}

void FluidSimulationGPU::initSimulation() {
	FluidSimulation::initSimulation();

	if (applyForcesShader.ID == 0) {
		applyForcesShader.CreateShader("compute_shaders/simulation_apply_forces.comp");
	}

	if (densityRelaxationShader.ID == 0) {
		densityRelaxationShader.CreateShader("compute_shaders/simulation_density_relaxation.comp");
	}

	if (updatePositionsShader.ID == 0) {
		updatePositionsShader.CreateShader("compute_shaders/simulation_update_positions.comp");
	}

	if (spatialHashGridKeysShader.ID == 0) {
		spatialHashGridKeysShader.CreateShader("compute_shaders/spatial_hash_grid_keys.comp");
	}

	if (spatialHashGridReorderShader.ID == 0) {
		spatialHashGridReorderShader.CreateShader("compute_shaders/spatial_hash_grid_reorder.comp");
	}

	if (spatialHashGridBoundsShader.ID == 0) {
		spatialHashGridBoundsShader.CreateShader("compute_shaders/spatial_hash_grid_bounds.comp");
	}

	if (radixCountShader.ID == 0) {
		radixCountShader.CreateShader("compute_shaders/radix_count.comp");
	}

	if (radixScanShader.ID == 0) {
		radixScanShader.CreateShader("compute_shaders/radix_scan.comp");
	}

	if (radixScatterShader.ID == 0) {
		radixScatterShader.CreateShader("compute_shaders/radix_scatter.comp");
	}

	unsigned int tableSize = 2 * numOfParticles;

	// simulation buffers
	initShaderBuffer<glm::vec3>(ssboPositions, &positions, numOfParticles);
	initShaderBuffer<glm::vec3>(ssboVelocities, &velocities, numOfParticles);
	initShaderBuffer<glm::vec3>(ssboPredictedPositions, nullptr, numOfParticles);
	initShaderBuffer<glm::vec3>(ssboDeltas, nullptr, numOfParticles);
	initShaderBuffer<float>(ssboDensities, nullptr, numOfParticles);
	initShaderBuffer<float>(ssboNearDensities, nullptr, numOfParticles);

	// spatial hash grid buffers
	initShaderBuffer<unsigned int>(ssboGridParticleKeys, nullptr, numOfParticles);
	initShaderBuffer<unsigned int>(ssboGridParticleValues, nullptr, numOfParticles);
	initShaderBuffer<unsigned int>(ssboCellStart, nullptr, tableSize);
	initShaderBuffer<unsigned int>(ssboCellEnd, nullptr, tableSize);

	initShaderBuffer<glm::vec3>(ssboSortedPositions, nullptr, numOfParticles);
	initShaderBuffer<glm::vec3>(ssboSortedVelocities, nullptr, numOfParticles);

	// radix sort buffers
	initShaderBuffer<unsigned int>(ssboRadixTempKeys, nullptr, numOfParticles);
	initShaderBuffer<unsigned int>(ssboRadixTempValues, nullptr, numOfParticles);
	unsigned int numBlocks = (numOfParticles + COMPUTE_SHADER_BLOCK_SIZE - 1) / COMPUTE_SHADER_BLOCK_SIZE;
	initShaderBuffer<unsigned int>(ssboRadixHistograms, nullptr, numBlocks * RADIX_SORT_BUCKET_COUNT);

}

void FluidSimulationGPU::bindShaderBuffers() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVelocities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboPredictedPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboDensities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboNearDensities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboDeltas);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboGridParticleKeys);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboGridParticleValues);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboCellStart);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, ssboCellEnd);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, ssboSortedPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboSortedVelocities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboRadixHistograms);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboRadixTempKeys);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, ssboRadixTempValues);
}

void FluidSimulationGPU::dispatchCurrentShader(unsigned int n) {
	unsigned int workSize = COMPUTE_SHADER_WORK_GROUP_SIZE;
	glDispatchCompute((n + workSize - 1) / workSize, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void FluidSimulationGPU::updateSimulation(unsigned int n, float dt) {
	unsigned int tableSize = 2 * numOfParticles;

	bindShaderBuffers();
	for (unsigned int i = 0; i < n; i++) {
		createSpatialHashGrid(numOfParticles, tableSize, false);
	}

	updateData();
}

void FluidSimulationGPU::updateData() {
	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPositions);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOfParticles * sizeof(glm::vec3), positions.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVelocities);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOfParticles * sizeof(glm::vec3), velocities.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void FluidSimulationGPU::createSpatialHashGrid(unsigned int numberOfParticles, unsigned int tableSize, bool usePredictedPositions) {
	GLuint clearFlag = 0xFFFFFFFF;
	glClearNamedBufferSubData(ssboCellStart, GL_R32UI, 0, tableSize * sizeof(unsigned int), GL_RED_INTEGER, GL_UNSIGNED_INT, &clearFlag);
	glClearNamedBufferSubData(ssboCellEnd, GL_R32UI, 0, tableSize * sizeof(unsigned int), GL_RED_INTEGER, GL_UNSIGNED_INT, &clearFlag);

	spatialHashGridKeysShader.use();
	spatialHashGridKeysShader.setUInt("tableSize", tableSize);
	spatialHashGridKeysShader.setFloat("spacing", smoothingRadius);
	spatialHashGridKeysShader.setUInt("numberOfParticles", numberOfParticles);
	spatialHashGridKeysShader.setInt("usePredictedPositions", usePredictedPositions ? 1 : 0);
	dispatchCurrentShader(numberOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	runGPURadixSort(numberOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	spatialHashGridReorderShader.use();
	spatialHashGridReorderShader.setUInt("numberOfParticles", numberOfParticles);
	dispatchCurrentShader(numberOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// ping pong
	std::swap(ssboPositions, ssboSortedPositions);
	std::swap(ssboVelocities, ssboSortedVelocities);

	spatialHashGridBoundsShader.use();
	spatialHashGridBoundsShader.setUInt("numberOfParticles", numberOfParticles);
	dispatchCurrentShader(numberOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void FluidSimulationGPU::runGPURadixSort(unsigned int numberOfParticles) {
	unsigned int numBlocks = (numberOfParticles + COMPUTE_SHADER_BLOCK_SIZE - 1) / COMPUTE_SHADER_BLOCK_SIZE;

	for (unsigned int shift = 0; shift < 32; shift += 4) {
		radixCountShader.use();
		radixCountShader.setUInt("numberOfParticles", numberOfParticles);
		radixCountShader.setUInt("shift", shift);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboGridParticleKeys);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboRadixHistograms);

		glDispatchCompute(numBlocks, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		radixScanShader.use();
		radixScanShader.setUInt("numBlocks", numBlocks);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboRadixHistograms);

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		radixScatterShader.use();
		radixScatterShader.setUInt("numberOfParticles", numberOfParticles);
		radixScatterShader.setUInt("shift", shift);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboGridParticleKeys);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboGridParticleValues);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboRadixHistograms);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboRadixTempKeys);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, ssboRadixTempValues);

		glDispatchCompute(numBlocks, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		std::swap(ssboGridParticleKeys, ssboRadixTempKeys);
		std::swap(ssboGridParticleValues, ssboRadixTempValues);
	}
}

void FluidSimulationGPU::applyForces(unsigned int numberOfParticles, float dt) {
	applyForcesShader.use();
	applyForcesShader.setFloat("dt", dt);
	applyForcesShader.setFloat("ViscosityMultiplier", viscosityMultiplier);
	applyForcesShader.setFloat("smoothingRadius", smoothingRadius);
	applyForcesShader.setFloat("particleMass", particleMass);
	glUniform3fv(glGetUniformLocation(applyForcesShader.ID, "gravity"), 1, &gravitationalForce[0]);
	glUniform4fv(glGetUniformLocation(applyForcesShader.ID, "planes"), 6, glm::value_ptr(container.getPlanesData()[0]));
	applyForcesShader.setFloat("particleRadius", particleRadius);

	dispatchCurrentShader(numberOfParticles);
}

void FluidSimulationGPU::computeDensities(unsigned int numberOfParticles, float dt) {
	densityRelaxationShader.use();
	densityRelaxationShader.setFloat("dt", dt);
	densityRelaxationShader.setFloat("smoothingRadius", smoothingRadius);
	densityRelaxationShader.setFloat("particleMass", particleMass);
	densityRelaxationShader.setFloat("targetDensity", targetDensity);
	densityRelaxationShader.setFloat("pressureMultiplier", pressureMultiplier);
	densityRelaxationShader.setFloat("nearPressureMultiplier", nearPressureMultiplier);
	glUniform4fv(glGetUniformLocation(densityRelaxationShader.ID, "planes"), 6, glm::value_ptr(container.getPlanesData()[0]));
	densityRelaxationShader.setFloat("particleRadius", particleRadius);

	dispatchCurrentShader(numberOfParticles);
}

void FluidSimulationGPU::updatePositions(unsigned int numberOfParticles, float dt) {
	updatePositionsShader.use();
	updatePositionsShader.setFloat("dt", dt);
	glUniform4fv(glGetUniformLocation(updatePositionsShader.ID, "planes"), 6, glm::value_ptr(container.getPlanesData()[0]));
	updatePositionsShader.setFloat("u_ParticleRadius", particleRadius);

	dispatchCurrentShader(numberOfParticles);
}