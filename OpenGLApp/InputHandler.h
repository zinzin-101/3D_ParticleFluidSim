#pragma once
#include <map>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputHandler {
private:
	GLFWwindow* window;
	glm::vec2 mousePosition;
	glm::vec2 lastMousePosition;
	glm::vec2 mouseOffset;

public:
	std::map<unsigned int, bool> keyDownMap;
	std::map<unsigned int, bool> mouseDownMap;

	InputHandler();
	void init(GLFWwindow* window);
	void update();
	bool getKeyDown(unsigned int key);
	bool getKey(unsigned int key);
	bool getMouseDown(unsigned int button);
	bool getMouse(unsigned int button);
	void updateMousePosition(glm::vec2 position);
	glm::vec2 getMousePosition() const;
	glm::vec2 getMouseOffset() const;
};