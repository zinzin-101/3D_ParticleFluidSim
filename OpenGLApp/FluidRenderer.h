#pragma once
#include "Camera.h"
#include "SphereRenderer.h"
#include "PlaneRenderer.h"
#include "CubeRenderer.h"
#include "CubeMapManager.h"
#include "FluidRaymarcher.h"
#include <vector>

class FluidSimulation;
class FluidContainer;

class FluidRenderer {
private:
	Camera camera;

	SphereRenderer sphereRenderer;
	void renderBasic(FluidSimulation* simulation);

	FluidRaymarcher raymarcher;
	CubeMapManager cubeMapManager;
	CubeMapRenderer cubeMapRenderer;
	void renderRaymarching(FluidSimulation* simulation);

	CubeRenderer cubeRenderer;
	void visualizeCubicContainer(Camera* camera, FluidContainer* container, bool drawAsOutline = false);

	GLuint obstacleDepthFBO;
	GLuint obstacleDepthTexture;
	GLsizei depthTextureWidth;
	GLsizei depthTextureHeight;
	void createObstacleRenderBuffer(GLsizei width, GLsizei height);
	void renderObstaclesDepth(FluidSimulation* simulation);
	void renderObstacles(FluidSimulation* simulation);

public:
	enum RenderingMode {
		BASIC,
		RAYMARCHING
	};

	float renderScale;

	bool showContainer;
	bool drawContainerAsOutline;

	float containerOpacity;

	bool showEnvMap;
	RenderingMode renderingMode;
	glm::vec3 backgroundColor;

	FluidRenderer();
	void init();
	void update();
	void render(FluidSimulation* simulation);
	void cleanup(FluidSimulation* simulation);

	void setRenderDistance(float distance);

	void updateViewport(GLsizei width, GLsizei height);

	Camera* getCamera();
	FluidRaymarcher* getRaymarcher();
	CubeMapManager* getCubeMapManager();
};
