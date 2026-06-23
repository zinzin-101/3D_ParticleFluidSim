	#pragma once
#include <glm/glm.hpp>

namespace FluidSimulationConfig {
	constexpr float FIXED_DT = 1.0f / 60.0f;

	const glm::vec3 DEFAULT_GRAVITATIONAL_FORCE = glm::vec3(0.0f, -9.81f, 0.0f);
	constexpr float DEFAULT_RENDER_SCALE = 1.0f;
	constexpr float DEFAULT_RENDER_DISTANCE = 100.0f;
	constexpr float DEFAULT_PARTICLE_RADIUS = 0.3f;

	constexpr float DEFAULT_VELOCITY_DAMPING = 0.985f;

	const glm::vec3 DEFAULT_CUBE_CONTAINER_ORIGIN = glm::vec3(0.0f, 0.0f, 0.0f);
	constexpr float DEFAULT_CUBE_CONTAINER_SIDE_LENGTH = 8.0f;

	constexpr float DEFAULT_PARTICLE_MASS = 1.0f;
	constexpr float DEFAULT_SMOOTHING_RADIUS = 1.0f;

	constexpr unsigned int DEFAULT_NUMBER_OF_PARTICLES = 1000;
	constexpr float DEFAULT_PARTICLE_SPACING = 0.6f;

	constexpr float DEFAULT_TARGET_DENSITY = 1.0f;
	constexpr float DEFAULT_TARGET_NEAR_DENSITY = 0.0f;
	constexpr float DEFAULT_PRESSURE_MULTIPLIER = 250.0f;
	constexpr float DEFAULT_NEAR_PRESSURE_MULTIPLIER = 50.0f;

	constexpr float DEFAULT_VISCOSITY = 0.005f;

	constexpr unsigned int SIMULATION_STEPS = 3;
	constexpr unsigned int MAX_SIMULATION_STEPS = 5;

	constexpr float PREDICTION_TIME = 1.0f / 120.0f;
}