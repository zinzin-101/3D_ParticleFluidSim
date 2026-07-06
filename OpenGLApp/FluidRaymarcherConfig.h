#pragma once

namespace FluidRaymarcherConfig {
	constexpr unsigned int DEFAULT_STEPS = 15;
	constexpr float DEFAULT_DENSITY_MULTIPLIER = 0.1f;

	constexpr float DEFAULT_AIR_REFRACTION_INDEX = 1.0003f;
	constexpr float DEFAULT_FLUID_REFRACTION_INDEX = 1.33f; // water

	const glm::vec3 DEFAULT_LIGHT_COLOR = glm::vec3(1.0f, 0.5f, 0.0f);

	constexpr float DEFAULT_ISO_LEVEL = 0.5f;

	constexpr float DEFAULT_SURFACE_SMOOTHING_RADIUS = 2.0f;
}