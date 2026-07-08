#pragma once
#include "Camera.h"
#include "SphereRenderer.h"
#include "CubeMapRenderer.h"
#include "FluidRaymarcher.h"
#include <vector>

class FluidSimulation;

class FluidRenderer {
private:
	Camera camera;
	SphereRenderer sphereRenderer;
	FluidRaymarcher raymarcher;
	CubeMapRenderer cubeMapRenderer;

	void renderBasic(FluidSimulation* simulation);
	void renderRaymarching(FluidSimulation* simulation);

public:
	enum RenderingMode {
		BASIC,
		RAYMARCHING
	};

	float renderScale;

	bool showContainer;
	bool drawContainerAsOutline;

	bool showEnvMap;
	RenderingMode renderingMode;
	glm::vec3 backgroundColor;

	FluidRenderer();
	void init();
	void update();
	void render(FluidSimulation* simulation);
	void cleanup(FluidSimulation* simulation);

	void setRenderDistance(float distance);

	Camera* getCamera();
	FluidRaymarcher* getRaymarcher();
};
