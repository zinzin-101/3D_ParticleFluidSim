#pragma once
#include <glm/glm.hpp>

namespace FluidSimulationConfig {
	constexpr float FIXED_DT = 1.0f / 60.0f;

	const glm::vec3 DEFAULT_GRAVITATIONAL_FORCE = glm::vec3(0.0f, -9.81f, 0.0f);
	constexpr float DEFAULT_RENDER_SCALE = 1.0f;
	constexpr float DEFAULT_PARTICLE_RADIUS = 0.05f;

	constexpr float DEFAULT_VELOCITY_DAMPING = 0.98f;

	const glm::vec3 DEFAULT_CUBE_CONTAINER_ORIGIN = glm::vec3(0.0f, 0.0f, 0.0f);
	constexpr float DEFAULT_CUBE_CONTAINER_SIDE_LENGTH = 7.0f;

	constexpr float DEFAULT_PARTICLE_MASS = 1.0f;
	constexpr float DEFAULT_SMOOTHING_RADIUS = 0.8f;

	constexpr unsigned int DEFAULT_NUMBER_OF_PARTICLES = 1000;
	constexpr float DEFAULT_PARTICLE_SPACING = 0.6f;

	constexpr float DEFAULT_TARGET_DENSITY = 6.0f;
	constexpr float DEFAULT_PRESSURE_MULTIPLIER = 500.0f;

	constexpr float DEFAULT_VISCOSITY = 0.5f;

	constexpr int SIMULATION_STEPS = 3;
	constexpr int MAX_SIMULATION_STEPS = 5;
}