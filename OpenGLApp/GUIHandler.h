#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class FluidSimulation;
class FluidRenderer;

class GUIHandler {
private:
	FluidSimulation* simulation;
	FluidRenderer* renderer;

public:
	void init(GLFWwindow* window, FluidSimulation* simulation);
	void update();
	void render();
	void cleanup();
};