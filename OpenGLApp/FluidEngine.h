#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "FluidSimulation.h"
#include "GUIHandler.h"
#include "InputHandler.h"
#include "Camera.h"

struct GLFWWindow;

class FluidEngine {
private:
	GLFWwindow* window;
	FluidSimulation simulation;
	GUIHandler gui;
	InputHandler input;
	Camera camera;
	float deltaTime;
	float lastTimeElapsed;

	glm::vec2 screenDimension;

	static FluidEngine* instance;
	
	void initWindow();
	void initGL();
	void update();
	void render();
	void cleanup();

public:

	FluidEngine();
	~FluidEngine();
	void init();
	void run();

	void toggleFullScreen();
	void setEnableCursor(bool value);

	InputHandler& getInputHandler();
	GUIHandler& getGUIHandler();
	FluidSimulation& getSimulation();
	Camera* getCamera();
	glm::vec2 getScreenDimension() const;
	float getDeltaTime() const;

	static FluidEngine* getInstance();

	static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void processInput(GLFWwindow* window);
};