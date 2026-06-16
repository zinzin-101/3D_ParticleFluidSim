#pragma once

namespace SphereRendererConfig {
	constexpr unsigned int SPHERE_SEGMENTS = 32;
	constexpr unsigned SPHERE_RINGS = 16;
	constexpr unsigned int SPHERE_QUADS_COUNTS = (SPHERE_RINGS + 1) * (SPHERE_SEGMENTS + 1);
	constexpr unsigned int SPHERE_INDICES_COUNT = SPHERE_RINGS * SPHERE_SEGMENTS;
	constexpr unsigned int MAX_INSTANCES = 50000000;
	const glm::vec3 DEFAULT_SPHERE_COLOR = glm::vec3(1.0f);
}