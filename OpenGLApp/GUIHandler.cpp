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
	ImGui::SameLine();
	if (ImGui::Button("Reset##pr")) {
		simulation->particleRadius = DEFAULT_PARTICLE_RADIUS;
		particleRadius = simulation->particleRadius;
	}

	static float smoothingRadius = simulation->smoothingRadius;
	if (ImGui::SliderFloat("Smoothing radius", &smoothingRadius, 0.01f, 10.0f, "%.2f")) {
		simulation->smoothingRadius = smoothingRadius;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##sr")) {
		simulation->smoothingRadius = DEFAULT_SMOOTHING_RADIUS;
		smoothingRadius = simulation->smoothingRadius;
	}

	static float targetDensity = simulation->targetDensity;
	if (ImGui::SliderFloat("Target density", &targetDensity, 0.01f, 10.0f, "%.2f")) {
		simulation->targetDensity = targetDensity;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##td")) {
		simulation->targetDensity = DEFAULT_TARGET_DENSITY;
		targetDensity = simulation->targetDensity;
	}

	static float pressureMultiplier = simulation->pressureMultiplier;
	if (ImGui::SliderFloat("Pressure multiplier", &pressureMultiplier, 0.01f, 1000.0f, "%.2f")) {
		simulation->pressureMultiplier = pressureMultiplier;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##pm")) {
		simulation->pressureMultiplier = DEFAULT_PRESSURE_MULTIPLIER;
		pressureMultiplier = simulation->pressureMultiplier;
	}

	static float viscosityMultiplier = simulation->viscosityMultiplier;
	if (ImGui::SliderFloat("Viscosity", &viscosityMultiplier, 0.0f, 2.0f, "%.2f")) {
		simulation->viscosityMultiplier = viscosityMultiplier;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##v")) {
		simulation->viscosityMultiplier = DEFAULT_VISCOSITY;
		viscosityMultiplier = simulation->viscosityMultiplier;
	}

	static float renderScale = renderer->renderScale;
	if (ImGui::SliderFloat("Render scale", &renderScale, 0.01f, 2.0f, "%.2f")) {
		renderer->renderScale = renderScale;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##rs")) {
		renderer->renderScale = 1.0f;
		renderScale = renderer->renderScale;
	}

	glm::vec3 gravity = simulation->gravitationalForce;
	if (ImGui::InputFloat3("Gravity", &gravity[0])) {
		simulation->gravitationalForce = gravity;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##g")) {
		simulation->gravitationalForce = DEFAULT_GRAVITATIONAL_FORCE;
		gravity = simulation->gravitationalForce;
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

	if (ImGui::Button("Look at container")) {
		glm::vec3 targetPos = simulation->getContainer()->getCurrentPosition();
		Camera* cam = engine->getCamera();
		cam->setForward(targetPos - cam->transform.position);
	}

	ImGui::Text("Container Transform:");
	ImGui::SameLine();
	if (ImGui::Button("Reset##ct")) {
		container->reset();
	}
	float dt = engine->getDeltaTime();
	static float containerTranslation[3] = { 0.0f, 0.0f, 0.0f };
	if (ImGui::DragFloat3("Translate", containerTranslation, 1.0f, 0.0f, 0.0f)) {
		container->translates(glm::vec3(containerTranslation[0], containerTranslation[1], containerTranslation[2]) * dt);
		containerTranslation[0] = containerTranslation[1] = containerTranslation[2] = 0.0f;
	}

	static float containerScale[3] = { 0.0f, 0.0f, 0.0f };
	if (ImGui::DragFloat3("Scale", containerScale, 1.0f, 0.0f, 0.0f)) {
		container->scales(glm::vec3(containerScale[0], containerScale[1], containerScale[2]) * dt);
		containerScale[0] = containerScale[1] = containerScale[2] = 0.0f;
	}
		
	static float containerRotation[3] = { 0.0f, 0.0f, 0.0f };
	if (ImGui::DragFloat3("Rotate", containerRotation, 1.0f, 0.0f, 0.0f)) {
		if (std::abs(containerRotation[0]) > 0.01f) {
			container->rotates(containerRotation[0], glm::vec3(1.0f, 0.0f, 0.0f) * dt);
		}
		if (std::abs(containerRotation[1]) > 0.01f) {
			container->rotates(containerRotation[1], glm::vec3(0.0f, 1.0f, 0.0f) * dt);
		}
		if (std::abs(containerRotation[2]) > 0.01f) {
			container->rotates(containerRotation[2], glm::vec3(0.0f, 0.0f, 1.0f) * dt);
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
	ImGui::SameLine();
	if (ImGui::Button("Reset##np")) {
		numOfParticles = DEFAULT_NUMBER_OF_PARTICLES;
	}

	static float spacing = simulation->particleSpacing;
	ImGui::SliderFloat("Particle spacing", &spacing, 0.001f, 1.0f);
	ImGui::SameLine();
	if (ImGui::Button("Reset##ps")) {
		spacing = DEFAULT_PARTICLE_SPACING;
	}

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