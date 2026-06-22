#pragma once
#include "Camera.h"
#include "SphereRenderer.h"
#include "Particle.h"
#include <vector>

class FluidRenderer {
private:
	Camera camera;
	SphereRenderer sphereRenderer;

public:
	float renderScale;

	FluidRenderer();
	void init();
	void update();
	void render(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& velocities, float radius, bool useInstancing = true);

	void setRenderDistance(float distance);

	Camera* getCamera();
};