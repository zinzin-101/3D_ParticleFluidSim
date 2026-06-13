#pragma once
#include "FluidRenderer.h"
#include "Camera.h"

class FluidSimulation {
private:
	FluidRenderer renderer;

public:
	void init();
	void update(float dt);
	void render(Camera* camera);
};