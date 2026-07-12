#pragma once
#include "Camera.h"
#include "SphereRenderer.h"
#include "CubeMapManager.h"
#include "FluidRaymarcher.h"
#include <vector>

class FluidSimulation;

class FluidRenderer {
private:
	Camera camera;
	SphereRenderer sphereRenderer;
	FluidRaymarcher raymarcher;
	CubeMapManager cubeMapManager;
	CubeMapRenderer cubeMapRenderer;

	void renderBasic(FluidSimulation* simulation);
	void renderRaymarching(FluidSimulation* simulation);

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
