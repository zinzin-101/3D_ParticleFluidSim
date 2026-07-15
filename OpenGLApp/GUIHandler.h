#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class FluidEngine;

class GUIHandler {
private:
	FluidEngine* engine;

	void handleSimulationSettingsGUI();
	void handleSettingsGUI();

	int currentSelectedObstacleIndex;

public:
	GUIHandler();
	void init(FluidEngine* engine);
	void update();
	void render();
	void cleanup();

	int getCurrentSelectedObstacleIndex() const;
};