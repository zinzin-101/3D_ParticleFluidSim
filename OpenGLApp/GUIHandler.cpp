#include "GUIHandler.h"
#include "FluidEngine.h"
#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"

using namespace FluidSimulationConfig;

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

	FluidEngine* engine = FluidEngine::getInstance();
	FluidContainer* container = simulation->getContainer();


	ImGui::Begin("Simulation");
	static int fpsIterationCount = 0;
	static float deltaTime = 0.0f;
	static float currentAvgFPS = 0.0f;
	if (fpsIterationCount > 10) {
		currentAvgFPS = 1.0f / (deltaTime / (float)fpsIterationCount);
		fpsIterationCount = 0;
		deltaTime = 0.0f;
	}
	deltaTime += engine->getDeltaTime();
	fpsIterationCount++;
	ImGui::Text("FPS: %.2f", currentAvgFPS);

	static bool isVsyncOn = engine->getIsVSyncOn();
	if (ImGui::Checkbox("VSync", &isVsyncOn)) {
		engine->setVSyncOn(isVsyncOn);
	}

	static float particleRadius = simulation->particleRadius;
	if (ImGui::SliderFloat("Particle radius", &particleRadius, 0.01f, 1.0f, "%.2f")) {
		simulation->particleRadius = particleRadius;
	}

	static float renderScale = renderer->renderScale;
	if (ImGui::SliderFloat("Render scale", &renderScale, 0.01f, 2.0f, "%.2f")) {
		renderer->renderScale = renderScale;
	}

	glm::vec3 gravity = simulation->gravitationalForce;
	if (ImGui::InputFloat3("Gravity", &gravity[0])) {
		simulation->gravitationalForce = gravity;
	}

	static bool resetGravityOnReset = false;
	static bool resetContainerOnReset = false;
	if (ImGui::Button("Reset Simulation")) {
		simulation->reset();

		if (resetGravityOnReset) {
			simulation->gravitationalForce = DEFAULT_GRAVITATIONAL_FORCE;
			gravity = simulation->gravitationalForce;
		}

		if (resetContainerOnReset) {
			container->reset();
		}
	}
	ImGui::Checkbox("Reset gravity on reset", &resetGravityOnReset);
	ImGui::Checkbox("Reset container on reset", &resetContainerOnReset);

	static bool pauseSimulation = false;
	pauseSimulation = simulation->pause;
	if (ImGui::Checkbox("Pause", &pauseSimulation)) {
		simulation->pause = pauseSimulation;
	}

	ImGui::Text("Container Transform:");
	static float containerTranslation[3] = { 0.0f, 0.0f, 0.0f };
	if (ImGui::DragFloat3("Translate", containerTranslation, 1.0f, 0.0f, 0.0f)) {
		container->translates(glm::vec3(containerTranslation[0], containerTranslation[1], containerTranslation[2]));
		containerTranslation[0] = containerTranslation[1] = containerTranslation[2] = 0.0f;
	}

	static float containerScale[3] = { 0.0f, 0.0f, 0.0f };
	if (ImGui::DragFloat3("Scale", containerScale, 1.0f, 0.0f, 0.0f)) {
		container->scales(glm::vec3(containerScale[0], containerScale[1], containerScale[2]));
		containerScale[0] = containerScale[1] = containerScale[2] = 0.0f;
	}
		
	static float containerRotation[3] = { 0.0f, 0.0f, 0.0f };
	if (ImGui::DragFloat3("Rotate", containerRotation, 1.0f, 0.0f, 0.0f)) {
		if (std::abs(containerRotation[0]) > 0.01f) {
			container->rotates(containerRotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
		}
		if (std::abs(containerRotation[1]) > 0.01f) {
			container->rotates(containerRotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (std::abs(containerRotation[2]) > 0.01f) {
			container->rotates(containerRotation[2], glm::vec3(0.0f, 0.0f, 1.0f));
		}
		containerRotation[0] = containerRotation[1] = containerRotation[2] = 0.0f;
	}

	static bool showContainer = simulation->showContainer;
	if (ImGui::Checkbox("Show container", &showContainer)) {
		simulation->showContainer = showContainer;
	}

	if (showContainer) {
		static float containerOpacity = container->planeOpacity;
		if (ImGui::SliderFloat("Container opacity", &containerOpacity, 0.001f, 1.0f, "%.2f")) {
			container->planeOpacity = containerOpacity;
			container->planeOpacity = containerOpacity;
		}
	}

	ImGui::Text("Initial parameters");
	static int numOfParticles = (int)simulation->numOfParticles;
	ImGui::SliderInt("Number of particles", &numOfParticles, 0, 1000000);
	static float spacing = simulation->particleSpacing;
	ImGui::SliderFloat("Particle spacing", &spacing, 0.001f, 1.0f);
	if (ImGui::Button("Apply initial parameters")) {
		simulation->numOfParticles = numOfParticles;
		simulation->particleSpacing = spacing;
		simulation->reset();
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