#include "FluidEngine.h"
#include "FluidEngineConfig.h"
#include <iostream>
#include <stdexcept>

using namespace FluidEngineConfig;

FluidEngine::FluidEngine() : window(nullptr), deltaTime(0.0f), lastTimeElapsed(0.0f), screenDimension() {
	init();
}

void FluidEngine::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	screenDimension = glm::vec2(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);

	if (INIT_IN_FULL_SCREEN) {
		screenDimension = glm::vec2(mode->width, mode->height);
		window = glfwCreateWindow(screenDimension.x, screenDimension.y, WINDOW_NAME, primaryMonitor, NULL);
	}
	else {
		window = glfwCreateWindow(screenDimension.x, screenDimension.y, WINDOW_NAME, NULL, NULL);
		glfwSetWindowPos(window, (mode->width / 2) - (screenDimension.x / 2), (mode->height / 2) - (screenDimension.y / 2));
	}

	
	if (window == NULL) {
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	}

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(DEFAULT_ENABLE_VSYNC ? 1 : 0);
}
void FluidEngine::initGL() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize GLAD");
	}
}

void FluidEngine::update() {
	gui.update();
	simulation.update(deltaTime);
}

void FluidEngine::render() {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	simulation.render();
	gui.render();
}

void FluidEngine::cleanup() {
	gui.cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void FluidEngine::init() {
	initWindow();
	input.init(window);
	initGL();
	gui.init(window);
}

void FluidEngine::run() {
	while (!glfwWindowShouldClose(window)) {
		float currentTimeElapsed = static_cast<float>(glfwGetTime());
		deltaTime = currentTimeElapsed - lastTimeElapsed;
		lastTimeElapsed = currentTimeElapsed;

		processInput(window);
		input.update();
		update();
		render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void FluidEngine::toggleFullScreen() {
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	if (glfwGetWindowMonitor(window)) { // full screen -> windowed
		screenDimension = glm::vec2(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
		glfwSetWindowMonitor(
			window, NULL, (mode->width / 2) - (DEFAULT_SCREEN_WIDTH / 2), (mode->height / 2) - (DEFAULT_SCREEN_HEIGHT / 2), 
			DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 
			GLFW_DONT_CARE);
	}
	else { // windowed -> full screen
		screenDimension = glm::vec2(mode->width, mode->height);
		glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
}

InputHandler& FluidEngine::getInputHandler() {
	return input;
}

GUIHandler& FluidEngine::getGUIHandler() {
	return gui;
}

FluidSimulation& FluidEngine::getSimulation() {
	return simulation;
}

void FluidEngine::frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void FluidEngine::mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	FluidEngine* engine = static_cast<FluidEngine*>(glfwGetWindowUserPointer(window));
	if (engine != nullptr) {
		engine->getInputHandler().updateMousePosition(glm::vec2(xpos, ypos));
	}
}

void FluidEngine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	processInput(window);
}

void FluidEngine::processInput(GLFWwindow* window) {
	FluidEngine* engine = static_cast<FluidEngine*>(glfwGetWindowUserPointer(window));
	if (engine != nullptr) {
		InputHandler& input = engine->getInputHandler();
		if (input.getKeyDown(GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, true);
		}

		// toggle fullscreen
		if (input.getKeyDown(GLFW_KEY_F11)) {
			engine->toggleFullScreen();
		}
	}

}