#include "FluidEngine.h"
#include "FluidEngineConfig.h"
#include "FluidSimulationConfig.h"
#include <iostream>
#include <stdexcept>

using namespace FluidEngineConfig;

FluidEngine* FluidEngine::instance = nullptr;

FluidEngine::FluidEngine() :
	window(nullptr),
	deltaTime(0.0f),
	lastTimeElapsed(0.0f),
	screenDimension(),
	isVSyncOn(false),
	mouseSensitivity(DEFAULT_MOUSE_SENSITIVITY),
	mouseScrollSensitivity(DEFAULT_MOUSE_SCROLL_SENSITIVITY)
{
	if (instance != nullptr) {
		throw std::runtime_error("Trying to create a new engine instance with already existing instance");
	}

	instance = this;
	init();
}

FluidEngine::~FluidEngine() {
	cleanup();
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
	glfwSetScrollCallback(window, mouseScrollCallback);

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

	instance = nullptr;
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
	switch (value) {
		case true:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;

		case false:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
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

void FluidEngine::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	FluidEngine* engine = static_cast<FluidEngine*>(glfwGetWindowUserPointer(window));
	if (engine != nullptr) {
		engine->getInputHandler()->updateMouseScrollOffset((float)yoffset);
		processInput(window);
	}
}

void FluidEngine::processInput(GLFWwindow* window) {
	FluidEngine* engine = static_cast<FluidEngine*>(glfwGetWindowUserPointer(window));
	if (engine != nullptr) {
		InputHandler& input = *engine->getInputHandler();

		// exit application
		if (input.getKeyDown(GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, true);
		}

		// toggle fullscreen
		if (input.getKeyDown(GLFW_KEY_F11)) {
			engine->toggleFullScreen();
		}

		// toggle pause
		if (input.getKeyDown(GLFW_KEY_SPACE)) {
			engine->getSimulation()->pause = !engine->getSimulation()->pause;
		}

		Camera* camera = engine->getCamera();
		float dt = engine->getDeltaTime();

		// camera rotation
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

		// camera movement
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

		// container transformation
		bool keyTDown = input.getKey(GLFW_KEY_T);
		bool keyRDown = input.getKey(GLFW_KEY_R);
		bool keyFDown = input.getKey(GLFW_KEY_F);
		if ((keyTDown || keyRDown || keyFDown) && !input.getMouse(GLFW_MOUSE_BUTTON_RIGHT)) 
		{
			glm::vec3 forward = glm::normalize(camera->getFoward());
			glm::vec3 right = glm::normalize(camera->getRight());
			glm::vec3 up = glm::normalize(camera->getUp());
			glm::vec2 mouseOffset = input.getMouseOffset() * engine->mouseSensitivity * 100.0f;
			float scrollOffset = input.getMouseScrollOffset() * engine->mouseScrollSensitivity;
			FluidContainer* container = engine->getSimulation()->getContainer();

			bool keyZDown = input.getKey(GLFW_KEY_Z); // X-axis
			bool keyXDown = input.getKey(GLFW_KEY_X); // Y-axis
			bool keyCDown = input.getKey(GLFW_KEY_C); // Z-axis

			// translation
			if (keyTDown) {
				glm::vec3 translation(0.0f);
				if (keyZDown) {
					translation = right * mouseOffset.x;
				}
				else if (keyXDown) {
					translation = up * -mouseOffset.y;
				}
				else if (keyCDown) {
					translation = forward * scrollOffset;
				}
				else {
					translation = right * mouseOffset.x + up * -mouseOffset.y + forward * scrollOffset;
				}
				container->translates(translation * FluidSimulationConfig::FIXED_DT);
			}
			// rotation
			else if (keyRDown) {
				if (keyZDown) {
					container->rotates(mouseOffset.x * FluidSimulationConfig::FIXED_DT, forward);
				}
				else if (keyXDown) {
					container->rotates(mouseOffset.y * FluidSimulationConfig::FIXED_DT, right);
				}
				else if (keyCDown) {
					container->rotates(scrollOffset* FluidSimulationConfig::FIXED_DT, up);
				}
				else {
					container->rotates(mouseOffset.x * FluidSimulationConfig::FIXED_DT, forward);
					container->rotates(mouseOffset.y * FluidSimulationConfig::FIXED_DT, right);
					container->rotates(scrollOffset * FluidSimulationConfig::FIXED_DT, up);
				}
			}
			// scale
			else if (keyFDown) {
				glm::vec3 currentContainerRotation = container->getCurrentRotation();
				float angleX = glm::radians(currentContainerRotation.x);
				float angleY = glm::radians(currentContainerRotation.y);
				float angleZ = glm::radians(currentContainerRotation.z);

				glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angleZ, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), angleY, glm::vec3(0.0f, 1.0f, 0.0f))
					* glm::rotate(glm::mat4(1.0f), angleX, glm::vec3(1.0f, 0.0f, 0.0f));

				glm::vec3 localX = glm::vec3(rotationMatrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
				glm::vec3 localY = glm::vec3(rotationMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
				glm::vec3 localZ = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

				float signX = (glm::dot(right, localX) >= 0.0f) ? 1.0f : -1.0f;
				float signY = (glm::dot(up, localY) >= 0.0f) ? 1.0f : -1.0f;
				float signZ = (glm::dot(forward, localZ) >= 0.0f) ? 1.0f : -1.0f;

				glm::vec3 scaling(0.0f);

				if (keyZDown) {
					glm::vec3 delta = right * mouseOffset.x;
					scaling = glm::vec3(glm::dot(localX, delta) * signX, 0.0f, 0.0f);
				}
				else if (keyXDown) {
					glm::vec3 delta = -mouseOffset.y * up;
					scaling = glm::vec3(0.0f, glm::dot(localY, delta) * signY, 0.0f);
				}
				else if (keyCDown) {
					glm::vec3 delta = scrollOffset * forward;
					scaling = glm::vec3(0.0f, 0.0f, glm::dot(localZ, delta) * signZ);
				}
				else {
					glm::vec3 totalDelta = (right * mouseOffset.x) + (-mouseOffset.y * up) + (scrollOffset * forward);
					scaling = glm::vec3(
						glm::dot(localX, totalDelta) * signX,
						glm::dot(localY, totalDelta) * signY,
						glm::dot(localZ, totalDelta) * signZ
					);
				}

				container->scales(scaling * FluidSimulationConfig::FIXED_DT);
			}
		}

		input.resetMouseScrollOffset();
	}
}