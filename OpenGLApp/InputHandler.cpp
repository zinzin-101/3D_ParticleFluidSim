#include "InputHandler.h"

InputHandler::InputHandler() : window(nullptr), mousePosition{}, lastMousePosition{}, mouseOffset{} {}

void InputHandler::init(GLFWwindow* window) {
	this->window = window;
}

bool InputHandler::getKeyDown(unsigned int key) {
	// init
	if (keyDownMap.count(key) == 0) {
		keyDownMap[key] = false;
		return false;
	}

	if (glfwGetKey(window, key) == GLFW_PRESS && keyDownMap.at(key)) {
		return false;
	}

	if (glfwGetKey(window, key) == GLFW_RELEASE && keyDownMap.at(key)) {
		keyDownMap[key] = false;
		return false;
	}

	if (glfwGetKey(window, key) == GLFW_PRESS && !keyDownMap.at(key)) {
		keyDownMap[key] = true;
		return true;
	}

	return false;
}

bool InputHandler::getKey(unsigned int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}

bool InputHandler::getMouseDown(unsigned int button) {
	// init
	if (mouseDownMap.count(button) == 0) {
		mouseDownMap[button] = false;
		return false;
	}

	if (glfwGetMouseButton(window, button) == GLFW_PRESS && mouseDownMap.at(button)) {
		return false;
	}

	if (glfwGetMouseButton(window, button) == GLFW_RELEASE && mouseDownMap.at(button)) {
		mouseDownMap[button] = false;
		return false;
	}

	if (glfwGetMouseButton(window, button) == GLFW_PRESS && !mouseDownMap.at(button)) {
		mouseDownMap[button] = true;
		return true;
	}

	return false;
}

bool InputHandler::getMouse(unsigned int button) {
	return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void InputHandler::update() {
	static bool firstMouseMovement = true;

	if (firstMouseMovement) {
		firstMouseMovement = false;
		lastMousePosition = mousePosition;
	}

	mouseOffset.x = mousePosition.x - lastMousePosition.x;
	mouseOffset.y = mousePosition.y - lastMousePosition.y; // reversed since y-coordinates go from bottom to top

	lastMousePosition = mousePosition;
}

void InputHandler::updateMousePosition(glm::vec2 position) {
	mousePosition = position;
}

void InputHandler::updateMouseScrollOffset(float yoffset) {
	mouseScrollOffset += yoffset;
}

void InputHandler::resetMouseScrollOffset() {
	mouseScrollOffset = 0.0f;
}

glm::vec2 InputHandler::getMousePosition() const {
	return glm::vec2(mousePosition.x, mousePosition.y);
}

glm::vec2 InputHandler::getMouseOffset() const {
	return mouseOffset;
}

float InputHandler::getMouseScrollOffset() const {
	return mouseScrollOffset;
}