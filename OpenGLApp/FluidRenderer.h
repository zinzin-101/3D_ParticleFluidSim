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
	void render(const std::vector<Particle>& particles, float radius, Camera* camera);
};