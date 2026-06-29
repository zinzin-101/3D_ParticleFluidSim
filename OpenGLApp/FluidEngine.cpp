#include "FluidEngine.h"
#include "FluidEngineConfig.h"
#include <iostream>
#include <stdexcept>

using namespace FluidEngineConfig;

FluidEngine* FluidEngine::instance = nullptr;

FluidEngine::FluidEngine(): 
	window(nullptr), 
	deltaTime(0.0f), 
	lastTimeElapsed(0.0f), 
	screenDimension(), 
	isVSyncOn(false), 
	mouseSensitivity(DEFAULT_MOUSE_SENSITIVITY) 
{
	if (instance != nullptr) {
		throw std::runtime_error("Trying to create a new engine instance with already existing instance");
	}

	instance = this;
	init();
}

FluidEngine::~FluidEngine() {
	cleanup();
	instance = nullptr;
}

void FluidEngine::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	screenDimension = glm::vec2(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);

	if (INIT_IN_FULL_SCREEN) {
		screenDimension = glm::vec2(mode->width, mode->height);
		window = glfwCreateWindow((int)screenDimension.x, (int)screenDimension.y, WINDOW_NAME, primaryMonitor, NULL);
	}
	else {
		window = glfwCreateWindow((int)screenDimension.x, (int)screenDimension.y, WINDOW_NAME, NULL, NULL);
		glfwSetWindowPos(window, (mode->width / 2) - ((int)screenDimension.x / 2), (mode->height / 2) - ((int)screenDimension.y / 2));
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
	isVSyncOn = DEFAULT_ENABLE_VSYNC;
	glfwSwapInterval(isVSyncOn ? 1 : 0);
}
void FluidEngine::initGL() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize GLAD");
	}
}

void FluidEngine::update() {
	gui.update();
	simulation.update(deltaTime);
	renderer.update();
}

void FluidEngine::render() {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.render(&simulation);
	gui.render();
}

void FluidEngine::cleanup() {
	renderer.cleanup(&simulation);
	gui.cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void FluidEngine::init() {
	initWindow();
	input.init(window);
	initGL();
	gui.init(this);

	simulation.init();
	renderer.init();
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

void FluidEngine::setEnableCursor(bool value) {
	glfwSetInputMode(window, GLFW_CURSOR, value ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void FluidEngine::setVSyncOn(bool value) {
	isVSyncOn = value;
	glfwSwapInterval(isVSyncOn ? 1 : 0);
}

GLFWwindow* FluidEngine::getWindow() {
	return window;
}

InputHandler* FluidEngine::getInputHandler() {
	return &input;
}

GUIHandler* FluidEngine::getGUIHandler() {
	return &gui;
}

FluidSimulation* FluidEngine::getSimulation() {
	return &simulation;
}

FluidRenderer* FluidEngine::getRenderer() {
	return &renderer;
}

Camera* FluidEngine::getCamera() {
	return renderer.getCamera();
}

glm::vec2 FluidEngine::getScreenDimension() const {
	return screenDimension;
}

bool FluidEngine::getIsVSyncOn() const {
	return isVSyncOn;
}

float FluidEngine::getDeltaTime() const {
	return deltaTime;
}

FluidEngine* FluidEngine::getInstance() {
	return instance;
}

void FluidEngine::frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void FluidEngine::mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	FluidEngine* engine = static_cast<FluidEngine*>(glfwGetWindowUserPointer(window));
	if (engine != nullptr) {
		engine->getInputHandler()->updateMousePosition(glm::vec2(xpos, ypos));
	}
}

void FluidEngine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	processInput(window);
}

void FluidEngine::processInput(GLFWwindow* window) {
	FluidEngine* engine = static_cast<FluidEngine*>(glfwGetWindowUserPointer(window));
	if (engine != nullptr) {
		InputHandler& input = *engine->getInputHandler();
		if (input.getKeyDown(GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, true);
		}

		// toggle fullscreen
		if (input.getKeyDown(GLFW_KEY_F11)) {
			engine->toggleFullScreen();
		}

		Camera* camera = engine->getCamera();
		float dt = engine->getDeltaTime();

		if (input.getMouse(GLFW_MOUSE_BUTTON_RIGHT)) {
			engine->setEnableCursor(false);

			glm::vec2 mouseOffset = input.getMouseOffset();
			camera->transform.eulerRotation.x -= mouseOffset.y * engine->mouseSensitivity;
			camera->transform.eulerRotation.y += mouseOffset.x * engine->mouseSensitivity;
			if (camera->transform.eulerRotation.x > 89.0f)
				camera->transform.eulerRotation.x = 89.0f;
			if (camera->transform.eulerRotation.x < -89.0f)
				camera->transform.eulerRotation.x = -89.0f;
		}
		else {
			engine->setEnableCursor(true);
		}

		float moveSpeed = 5.0f;
		glm::vec3 movement(0.0f);
		if (input.getKey(GLFW_KEY_W)) {
			movement += camera->getFoward();
		}
		if (input.getKey(GLFW_KEY_S)) {
			movement -= camera->getFoward();
		}
		if (input.getKey(GLFW_KEY_D)) {
			movement += camera->getRight();
		}
		if (input.getKey(GLFW_KEY_A)) {
			movement -= camera->getRight();
		}
		if (input.getKey(GLFW_KEY_E)) {
			movement += camera->getUp();
		}
		if (input.getKey(GLFW_KEY_Q)) {
			movement -= camera->getUp();
		}

		if (glm::length(movement) > 0.1f) {
			if (input.getKey(GLFW_KEY_LEFT_SHIFT)) {
				camera->transform.position += glm::normalize(movement) * moveSpeed * 10.0f * dt;
			}
			else {
				camera->transform.position += glm::normalize(movement) * moveSpeed * dt;
			}
		}
	}

}