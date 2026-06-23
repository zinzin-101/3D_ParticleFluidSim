#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class FluidEngine;

class GUIHandler {
private:
	FluidEngine* engine;

public:
	void init(GLFWwindow* window, FluidEngine* engine);
	void update();
	void render();
	void cleanup();
};