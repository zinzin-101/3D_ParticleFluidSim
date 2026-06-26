#include "FluidSimulationGPU.h"
#include "FluidSimulationConfig.h"

using namespace FluidSimulationConfig;

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

	if (ssboNearDensities != 0) {
		glDeleteBuffers(1, &ssboNearDensities);
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

	if (updatePositionsShader.ID = 0) {
		updatePositionsShader.CreateShader("compute_shaders/simulation_update_positions.comp");
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

	// simulation buffers
	initShaderBuffer<glm::vec3>(ssboPositions, &positions, numberOfParticles);
	initShaderBuffer<glm::vec3>(ssboVelocities, &velocities, numberOfParticles);
	initShaderBuffer<glm::vec3>(ssboPredictedPositions, nullptr, numberOfParticles);
	initShaderBuffer<glm::vec3>(ssboDeltas, nullptr, numberOfParticles);
	initShaderBuffer<float>(ssboDensities, nullptr, numberOfParticles);
	initShaderBuffer<float>(ssboNearDensities, nullptr, numberOfParticles);

	// spatial hash grid buffers
	initShaderBuffer<unsigned int>(ssboCellStart, nullptr, tableSize + 1);
	initShaderBuffer<unsigned int>(ssboCellEntries, nullptr, numberOfParticles);
	initShaderBuffer<unsigned int>(ssboCellLocalCounters, nullptr, tableSize + 1);
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

void FluidSimulationGPU::dispatchCurrentShader(unsigned int n) {
	unsigned int workSize = COMPUTE_SHADER_WORK_GROUP_SIZE;
	glDispatchCompute((n + workSize - 1) / workSize, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void FluidSimulationGPU::updateSimulation(unsigned int n, float dt) {
	unsigned int numberOfParticles = (unsigned int)positions.size();
	unsigned int tableSize = 2 * numberOfParticles;

	bindShaderBuffers();
	for (unsigned int i = 0; i < n; i++) {
		createSpatialHashGrid(numberOfParticles, tableSize, false);
	}

	updateData();
}

void FluidSimulationGPU::updateData() {
	unsigned int numberOfParticles = positions.size();

	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPositions);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numberOfParticles * sizeof(glm::vec3), positions.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVelocities);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numberOfParticles * sizeof(glm::vec3), velocities.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void FluidSimulationGPU::createSpatialHashGrid(unsigned int numberOfParticles, unsigned int tableSize, bool usePredictedPositions) {
	GLuint zero = 0;

	glClearNamedBufferSubData(ssboCellStart, GL_R32UI, 0, (tableSize + 1) * sizeof(unsigned int), GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
	spatialHashGridCountShader.use();
	spatialHashGridCountShader.setUInt("tableSize", tableSize);
	spatialHashGridCountShader.setFloat("spacing", smoothingRadius);
	spatialHashGridCountShader.setUInt("numberOfParticles", numberOfParticles);
	spatialHashGridCountShader.setInt("usePredictedPositions", usePredictedPositions);
	dispatchCurrentShader(numberOfParticles);

	spatialHashGridScanShader.use();
	spatialHashGridScanShader.setUInt("tableSize", tableSize);
	unsigned int workSize = COMPUTE_SHADER_WORK_GROUP_SIZE;
	dispatchCurrentShader(numberOfParticles);

	glCopyNamedBufferSubData(ssboCellStart, ssboCellLocalCounters, 0, 0, (tableSize + 1) * sizeof(unsigned int));

	spatialHashGridSortShader.use();
	spatialHashGridSortShader.setUInt("tableSize", tableSize);
	spatialHashGridSortShader.setFloat("spacing", smoothingRadius);
	spatialHashGridSortShader.setUInt("numberOfParticles", numberOfParticles);
	spatialHashGridSortShader.setInt("usePredictedPositions", usePredictedPositions);
	dispatchCurrentShader(numberOfParticles);
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