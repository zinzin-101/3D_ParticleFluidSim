#pragma once
#include <glm/glm.hpp>

namespace FluidSimulationConfig {
	const glm::vec3 DEFAULT_GRAVITATIONAL_FORCE = glm::vec3(0.0f, -9.81f, 0.0f);
	constexpr float DEFAULT_RENDER_SCALE = 1.0f;
	constexpr float DEFAULT_PARTICLE_RADIUS = 0.05f;
}