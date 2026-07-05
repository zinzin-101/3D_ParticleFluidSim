#include "FluidSimulationGPU.h"
#include "FluidSimulationConfig.h"

using namespace FluidSimulationConfig;

FluidSimulationGPU::FluidSimulationGPU() :
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
	ssboSortedPredictedPositions(0),
	ssboSortedVelocities(0),
	ssboSortedDensities(0),
	ssboSortedNearDensities(0),
	ssboSortedDeltas(0),
	ssboRadixTempKeys(0),
	ssboRadixTempValues(0),
	ssboRadixHistograms(0)
{}

FluidSimulationGPU::~FluidSimulationGPU() {
	if (ssboPositions != 0) {
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

	if (ssboSortedPredictedPositions != 0) {
		glDeleteBuffers(1, &ssboSortedPredictedPositions);
	}

	if (ssboSortedVelocities != 0) {
		glDeleteBuffers(1, &ssboSortedVelocities);
	}

	if (ssboSortedDensities != 0) {
		glDeleteBuffers(1, &ssboSortedDensities);
	}

	if (ssboSortedNearDensities != 0) {
		glDeleteBuffers(1, &ssboSortedNearDensities);
	}

	if (ssboSortedDeltas != 0) {
		glDeleteBuffers(1, &ssboSortedDeltas);
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

	if (densityShader.ID == 0) {
		densityShader.CreateShader("compute_shaders/simulation_density.comp");
	}

	if (densityRelaxationShader.ID == 0) {
		densityRelaxationShader.CreateShader("compute_shaders/simulation_density_relaxation.comp");
	}

	if (updateDeltasShader.ID == 0) {
		updateDeltasShader.CreateShader("compute_shaders/simulation_update_deltas.comp");
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
	initShaderBuffer<glm::vec3>(ssboPredictedPositions, &positions, numOfParticles);
	initShaderBuffer<unsigned int>(ssboDeltas, nullptr, numOfParticles * 3);
	initShaderBuffer<float>(ssboDensities, nullptr, numOfParticles);
	initShaderBuffer<float>(ssboNearDensities, nullptr, numOfParticles);

	// spatial hash grid buffers
	initShaderBuffer<unsigned int>(ssboGridParticleKeys, nullptr, numOfParticles);
	initShaderBuffer<unsigned int>(ssboGridParticleValues, nullptr, numOfParticles);
	initShaderBuffer<unsigned int>(ssboCellStart, nullptr, tableSize);
	initShaderBuffer<unsigned int>(ssboCellEnd, nullptr, tableSize);

	initShaderBuffer<glm::vec3>(ssboSortedPositions, &positions, numOfParticles);
	initShaderBuffer<glm::vec3>(ssboSortedPredictedPositions, &positions, numOfParticles);
	initShaderBuffer<glm::vec3>(ssboSortedVelocities, &velocities, numOfParticles);
	initShaderBuffer<float>(ssboSortedDensities, nullptr, numOfParticles);
	initShaderBuffer<float>(ssboSortedNearDensities, nullptr, numOfParticles);
	initShaderBuffer<unsigned int>(ssboSortedDeltas, nullptr, numOfParticles * 3);

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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboSortedPredictedPositions);
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
		createSpatialHashGrid(tableSize, false);
		applyForces(dt);

		for (unsigned int itr = 0; itr < RELAXATION_ITERATIONS; itr++) {
			createSpatialHashGrid(tableSize, true);
			computeDensities(dt);
			updateDeltas();
		}

		updatePositions(dt);
	}

	bindShaderBuffers();
	updateData();
}

void FluidSimulationGPU::updateData() {
	//glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPositions);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOfParticles * sizeof(glm::vec3), positions.data());

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVelocities);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOfParticles * sizeof(glm::vec3), velocities.data());

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDensities);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOfParticles * sizeof(float), densities.data());

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDeltas);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numOfParticles * sizeof(unsigned int) * 3, deltas.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void FluidSimulationGPU::createSpatialHashGrid(unsigned int tableSize, bool usePredictedPositions) {
	GLuint clearFlag = 0xFFFFFFFF;
	glClearNamedBufferSubData(ssboCellStart, GL_R32UI, 0, tableSize * sizeof(unsigned int), GL_RED_INTEGER, GL_UNSIGNED_INT, &clearFlag);
	glClearNamedBufferSubData(ssboCellEnd, GL_R32UI, 0, tableSize * sizeof(unsigned int), GL_RED_INTEGER, GL_UNSIGNED_INT, &clearFlag);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	spatialHashGridKeysShader.use();
	spatialHashGridKeysShader.setUInt("tableSize", tableSize);
	spatialHashGridKeysShader.setFloat("spacing", smoothingRadius);
	spatialHashGridKeysShader.setUInt("numberOfParticles", numOfParticles);
	spatialHashGridKeysShader.setInt("usePredictedPositions", usePredictedPositions ? 1 : 0);
	dispatchCurrentShader(numOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	runGPURadixSort(numOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	spatialHashGridReorderShader.use();
	spatialHashGridReorderShader.setUInt("numberOfParticles", numOfParticles);
	dispatchCurrentShader(numOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	std::swap(ssboPositions, ssboSortedPositions);
	std::swap(ssboVelocities, ssboSortedVelocities);
	std::swap(ssboPredictedPositions, ssboSortedPredictedPositions);
	std::swap(ssboDensities, ssboSortedDensities);
	std::swap(ssboNearDensities, ssboSortedNearDensities);
	std::swap(ssboDeltas, ssboSortedDeltas);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVelocities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboPredictedPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboDensities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboNearDensities);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, ssboSortedPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboSortedVelocities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboSortedPredictedPositions);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 18, ssboSortedDensities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, ssboSortedNearDensities);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, ssboSortedDeltas);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboGridParticleKeys);

	spatialHashGridBoundsShader.use();
	spatialHashGridBoundsShader.setUInt("numberOfParticles", numOfParticles);
	spatialHashGridBoundsShader.setUInt("tableSize", tableSize);
	dispatchCurrentShader(numOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void FluidSimulationGPU::runGPURadixSort(unsigned int numberOfParticles) {
	unsigned int numBlocks = (numberOfParticles + COMPUTE_SHADER_BLOCK_SIZE - 1) / COMPUTE_SHADER_BLOCK_SIZE;

	GLuint currentSrcKeys = ssboGridParticleKeys;
	GLuint currentSrcValues = ssboGridParticleValues;
	GLuint currentDstKeys = ssboRadixTempKeys;
	GLuint currentDstValues = ssboRadixTempValues;

	for (unsigned int shift = 0; shift < 32; shift += 4) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, currentSrcKeys);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, currentSrcValues);

		radixCountShader.use();
		radixCountShader.setUInt("numberOfParticles", numberOfParticles);
		radixCountShader.setUInt("shift", shift);

		glDispatchCompute(numBlocks, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		radixScanShader.use();
		radixScanShader.setUInt("numBlocks", numBlocks);

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		radixScatterShader.use();
		radixScatterShader.setUInt("numberOfParticles", numberOfParticles);
		radixScatterShader.setUInt("shift", shift);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, currentSrcKeys);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, currentSrcValues);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, currentDstKeys);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, currentDstValues);

		glDispatchCompute(numBlocks, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		std::swap(currentSrcKeys, currentDstKeys);
		std::swap(currentSrcValues, currentDstValues);
	}

	if (currentSrcKeys != ssboGridParticleKeys) {
		glCopyNamedBufferSubData(ssboRadixTempKeys, ssboGridParticleKeys, 0, 0, numberOfParticles * sizeof(unsigned int));
		glCopyNamedBufferSubData(ssboRadixTempValues, ssboGridParticleValues, 0, 0, numberOfParticles * sizeof(unsigned int));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

void FluidSimulationGPU::applyForces(float dt) {
	applyForcesShader.use();
	applyForcesShader.setUInt("tableSize", 2 * numOfParticles);
	applyForcesShader.setFloat("spacing", smoothingRadius);
	applyForcesShader.setUInt("numberOfParticles", numOfParticles);
	applyForcesShader.setFloat("dt", dt);
	applyForcesShader.setFloat("viscosityMultiplier", viscosityMultiplier);
	applyForcesShader.setFloat("smoothingRadius", smoothingRadius);
	applyForcesShader.setFloat("particleMass", particleMass);
	glUniform3fv(glGetUniformLocation(applyForcesShader.ID, "gravity"), 1, &gravitationalForce[0]);
	glUniform4fv(glGetUniformLocation(applyForcesShader.ID, "planes"), 6, glm::value_ptr(container.getPlanesData()[0]));
	applyForcesShader.setFloat("particleRadius", particleRadius);

	dispatchCurrentShader(numOfParticles);
}

void FluidSimulationGPU::computeDensities(float dt) {
	densityShader.use();
	densityShader.setUInt("numberOfParticles", numOfParticles);
	densityShader.setFloat("spacing", smoothingRadius);
	densityShader.setFloat("smoothingRadius", smoothingRadius);
	densityShader.setUInt("tableSize", 2 * numOfParticles);
	densityShader.setFloat("particleMass", particleMass);
	dispatchCurrentShader(numOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	densityRelaxationShader.use();
	GLuint zero = 0;
	glClearNamedBufferSubData(ssboDeltas, GL_R32UI, 0, numOfParticles * 3 * sizeof(unsigned int), GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	densityRelaxationShader.setUInt("tableSize", 2 * numOfParticles);
	densityRelaxationShader.setFloat("spacing", smoothingRadius);
	densityRelaxationShader.setUInt("numberOfParticles", numOfParticles);
	densityRelaxationShader.setFloat("smoothingRadius", smoothingRadius);
	densityRelaxationShader.setFloat("dt", dt);
	densityRelaxationShader.setFloat("targetDensity", targetDensity);
	densityRelaxationShader.setFloat("pressureMultiplier", pressureMultiplier);
	densityRelaxationShader.setFloat("nearPressureMultiplier", nearPressureMultiplier);
	dispatchCurrentShader(numOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void FluidSimulationGPU::updateDeltas() {
	updateDeltasShader.use();
	updateDeltasShader.setUInt("numberOfParticles", numOfParticles);
	updateDeltasShader.setFloat("particleRadius", particleRadius);
	glUniform4fv(glGetUniformLocation(updateDeltasShader.ID, "planes"), 6, glm::value_ptr(container.getPlanesData()[0]));
	dispatchCurrentShader(numOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void FluidSimulationGPU::updatePositions(float dt) {
	updatePositionsShader.use();
	updatePositionsShader.setUInt("numberOfParticles", numOfParticles);
	updatePositionsShader.setFloat("dt", dt);
	updatePositionsShader.setFloat("particleRadius", particleRadius);
	glUniform4fv(glGetUniformLocation(updatePositionsShader.ID, "planes"), 6, glm::value_ptr(container.getPlanesData()[0]));
	dispatchCurrentShader(numOfParticles);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

GLuint FluidSimulationGPU::getPositionsSSBO() const {
	return ssboPositions;
}

GLuint FluidSimulationGPU::getVelocitiesSSBO() const {
	return ssboVelocities;
}

GLuint FluidSimulationGPU::getDensitiesSSBO() const  {
	return ssboDensities;
}

GLuint FluidSimulationGPU::getCellStartSSBO() const {
	return ssboCellStart;
}

GLuint FluidSimulationGPU::getCellEndSSBO() const {
	return ssboCellEnd;
}