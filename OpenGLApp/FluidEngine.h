#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

//#include "FluidSimulation.h"
#include "FluidSimulationGPU.h"
#include "FluidRenderer.h"
#include "GUIHandler.h"
#include "InputHandler.h"

struct GLFWWindow;

class FluidEngine {
private:
	GLFWwindow* window;
	FluidSimulationGPU simulation;
	FluidRenderer renderer;
	GUIHandler gui;
	InputHandler input;
	float deltaTime;
	float lastTimeElapsed;

	glm::vec2 screenDimension;
	bool isVSyncOn;

	static FluidEngine* instance;
	
	void initWindow();
	void initGL();
	void update();
	void render();
	void cleanup();

public:
	float mouseSensitivity;

	FluidEngine();
	~FluidEngine();
	void init();
	void run();

	void toggleFullScreen();
	void setEnableCursor(bool value);
	void setVSyncOn(bool value);

	InputHandler* getInputHandler();
	GUIHandler* getGUIHandler();
	FluidSimulation* getSimulation();
	FluidRenderer* getRenderer();
	Camera* getCamera();
	glm::vec2 getScreenDimension() const;
	bool getIsVSyncOn() const;
	float getDeltaTime() const;

	static FluidEngine* getInstance();

	static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void processInput(GLFWwindow* window);
};