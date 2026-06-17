#pragma once
#include <glm/glm.hpp>

namespace FluidSimulationConfig {
	const glm::vec3 DEFAULT_GRAVITATIONAL_FORCE = glm::vec3(0.0f, -9.81f, 0.0f);
	constexpr float DEFAULT_RENDER_SCALE = 1.0f;
	constexpr float DEFAULT_PARTICLE_RADIUS = 0.05f;

	const glm::vec3 DEFAULT_CUBE_CONTAINER_ORIGIN = glm::vec3(0.0f, 2.5f, 0.0f);
	constexpr float DEFAULT_CUBE_CONTAINER_SIDE_LENGTH = 5.0f;

	constexpr unsigned int DEFAULT_NUMBER_OF_PARTICLES = 1000;
	constexpr float DEFAULT_PARTICLE_SPACING = DEFAULT_PARTICLE_RADIUS * 2.0f;

	constexpr float DEFAULT_PARTICLE_MASS = 1.0f;
	constexpr float DEFAULT_SMOOTHING_RADIUS = 5.0f;

	constexpr float DEFAULT_TARGET_DENSITY = 1.0f;
	constexpr float DEFAULT_PRESSURE_MULTIPLIER = 1.0f;
}