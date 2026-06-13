#include "GUIHandler.h"

void GUIHandler::init(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 440");
}

void GUIHandler::update() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Test");
	ImGui::Text("text test");
	static bool toggleTestText = false;
	if (ImGui::Button("button test")) {
		toggleTestText = !toggleTestText;
	}

	if (toggleTestText) {
		ImGui::Text("toggle test");
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