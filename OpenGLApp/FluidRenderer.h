#pragma once
#include "SphereRenderer.h"
#include "Particle.h"
#include <vector>

class FluidRenderer {
private:
	SphereRenderer sphereRenderer;

public:
	float renderScale;

	FluidRenderer();
	void init();
	void render(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& velocities, float radius, Camera* camera, bool useInstancing = true);
};