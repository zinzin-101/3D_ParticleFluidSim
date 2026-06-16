#include "GUIHandler.h"
#include "FluidSimulation.h"
#include "FluidRenderer.h"

void GUIHandler::init(GLFWwindow* window, FluidSimulation* simulation) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 440");

	this->simulation = simulation;
	this->renderer = simulation->getRenderer();
}

void GUIHandler::update() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Simulation");
	static float particleRadius = simulation->particleRadius;
	if (ImGui::SliderFloat("Particle radius", &particleRadius, 0.01f, 1.0f, "%.2f")) {
		simulation->particleRadius = particleRadius;
	}

	static float renderScale = renderer->renderScale;
	if (ImGui::SliderFloat("Render scale", &renderScale, 0.01f, 2.0f, "%.2f")) {
		renderer->renderScale = renderScale;
	}


	ImGui::End();
}

void GUIHandler::render() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIHandler::cleanup() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}