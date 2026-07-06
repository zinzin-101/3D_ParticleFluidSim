#pragma once

namespace FluidRaymarcherConfig {
	constexpr unsigned int DEFAULT_STEPS = 50;
	constexpr float DEFAULT_DENSITY_MULTIPLIER = 0.05f;

	constexpr float DEFAULT_AIR_REFRACTION_INDEX = 1.0003f;
	constexpr float DEFAULT_FLUID_REFRACTION_INDEX = 1.33f; // water

	const glm::vec3 DEFAULT_LIGHT_COLOR = glm::vec3(0.0f, 0.1f, 1.0f);

	constexpr float DEFAULT_ISO_LEVEL = 0.5f;
}