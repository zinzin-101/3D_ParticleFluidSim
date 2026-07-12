	#pragma once
#include <glm/glm.hpp>

namespace FluidSimulationConfig {
	constexpr float FIXED_DT = 1.0f / 60.0f;

	const glm::vec3 DEFAULT_GRAVITATIONAL_FORCE = glm::vec3(0.0f, -9.81f, 0.0f);
	const glm::vec3 DEFAULT_REACTIVE_GRAVITATIONAL_FORCE = glm::vec3(0.0f, -100.0f, 0.0f);
	constexpr float DEFAULT_RENDER_SCALE = 1.0f;
	constexpr float DEFAULT_RENDER_DISTANCE = 250.0f;
	constexpr float DEFAULT_PARTICLE_RADIUS = 0.6f;

	constexpr float DEFAULT_VELOCITY_DAMPING = 0.985f;

	const glm::vec3 DEFAULT_CUBE_CONTAINER_ORIGIN = glm::vec3(0.0f, 0.0f, 0.0f);
	constexpr float DEFAULT_CUBE_CONTAINER_SIDE_LENGTH = 25.0f;

	constexpr float DEFAULT_PARTICLE_MASS = 1.0f;
	constexpr float DEFAULT_SMOOTHING_RADIUS = 1.0f;

	constexpr unsigned int DEFAULT_NUMBER_OF_PARTICLES = 10000;
	constexpr float DEFAULT_PARTICLE_SPACING = 0.95f;

	constexpr float DEFAULT_TARGET_DENSITY = 1.0f;
	constexpr float DEFAULT_PRESSURE_MULTIPLIER = 500.0f;
	constexpr float DEFAULT_NEAR_PRESSURE_MULTIPLIER = 1500.0f;

	constexpr float DEFAULT_VISCOSITY = 5.0f;

	constexpr unsigned int SIMULATION_STEPS = 3;
	constexpr unsigned int RELAXATION_ITERATIONS = 2;
	constexpr unsigned int MAX_SIMULATION_STEPS = 5;

	constexpr float PREDICTION_TIME = 1.0f / 120.0f;

	constexpr float UPFLOW_MULTIPLIER = 0.1f;

	constexpr unsigned int COMPUTE_SHADER_WORK_GROUP_SIZE = 256;
	constexpr unsigned int COMPUTE_SHADER_BLOCK_SIZE = 256;
	constexpr unsigned int RADIX_SORT_BUCKET_COUNT = 16;

	const glm::vec3 DEFAULT_BACKGROUND_COLOR = glm::vec3(0.1f);

	constexpr unsigned int MAX_NUMBER_OF_OBSTACLES = 16;
	constexpr float DEFAULT_OBSTACLE_RADIUS = 5.0f;
}