#pragma once
#include "Camera.h"
#include "SphereRenderer.h"
#include "CubeMapRenderer.h"
//#include "Particle.h"
#include <vector>

class FluidSimulation;

class FluidRenderer {
private:
	Camera camera;
	SphereRenderer sphereRenderer;
	CubeMapRenderer cubeMapRenderer;

public:
	float renderScale;

	bool showContainer;
	bool drawContainerAsOutline;

	bool showEnvMap;

	FluidRenderer();
	void init();
	void update();
	void render(FluidSimulation* simulation, bool useInstancing = true);
	void cleanup(FluidSimulation* simulation);

	void setRenderDistance(float distance);

	Camera* getCamera();

	static unsigned int loadTexture(char const* path);
};
