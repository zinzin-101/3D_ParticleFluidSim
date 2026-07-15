#include "GUIHandler.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "FluidEngine.h"
#include "FluidEngineConfig.h"
#include "FluidSimulation.h"
#include "FluidSimulationConfig.h"
#include "FluidRaymarcherConfig.h"

using namespace FluidEngineConfig;
using namespace FluidSimulationConfig;

GUIHandler::GUIHandler(): engine(nullptr), currentSelectedObstacleIndex(0) {}

void GUIHandler::init(FluidEngine* engine) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(engine->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 440");

	this->engine = engine;
}

void GUIHandler::handleSimulationSettingsGUI() {
	FluidSimulation* simulation = engine->getSimulation();
	FluidContainer* container = simulation->getContainer();

	ImGui::Begin("Simulation settings");
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
	if (ImGui::SliderFloat("Pressure multiplier", &pressureMultiplier, 0.00f, 2000.0f, "%.2f")) {
		simulation->pressureMultiplier = pressureMultiplier;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##pm")) {
		simulation->pressureMultiplier = DEFAULT_PRESSURE_MULTIPLIER;
		pressureMultiplier = simulation->pressureMultiplier;
	}

	static float nearPressureMultiplier = simulation->nearPressureMultiplier;
	if (ImGui::SliderFloat("Near pressure multiplier", &nearPressureMultiplier, 0.0f, 2000.0f, "%.2f")) {
		simulation->nearPressureMultiplier = nearPressureMultiplier;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##npm")) {
		simulation->nearPressureMultiplier = DEFAULT_NEAR_PRESSURE_MULTIPLIER;
		nearPressureMultiplier = simulation->nearPressureMultiplier;
	}

	static float viscosityMultiplier = simulation->viscosityMultiplier;
	if (ImGui::SliderFloat("Viscosity", &viscosityMultiplier, 0.0f, 30.0f, "%.3f")) {
		simulation->viscosityMultiplier = viscosityMultiplier;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##v")) {
		simulation->viscosityMultiplier = DEFAULT_VISCOSITY;
		viscosityMultiplier = simulation->viscosityMultiplier;
	}

	static float mass = simulation->particleMass;
	if (ImGui::SliderFloat("Mass", &mass, 0.1f, 10.0f, "%.1f")) {
		simulation->particleMass = mass;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##m")) {
		simulation->particleMass = DEFAULT_PARTICLE_MASS;
		mass = simulation->particleMass;
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
	ImGui::SameLine();
	if (ImGui::Button("Reactive")) {
		simulation->gravitationalForce = DEFAULT_REACTIVE_GRAVITATIONAL_FORCE;
		gravity = simulation->gravitationalForce;
	}

	static float timeScale = simulation->timeScale;
	if (ImGui::SliderFloat("Time scale", &timeScale, 0.1f, 1.5f, "%.2f")) {
		simulation->timeScale = timeScale;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##ts")) {
		simulation->timeScale = 1.0f;
		timeScale = simulation->timeScale;
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

	// container translation
	static glm::vec3 containerTranslation = glm::vec3(0.0f);
	glm::vec3 containerPos = container->getCurrentPosition();
	ImGui::BeginGroup();
	ImGui::Text("Translate"); 
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.0f);
	std::string formatX = "X: " + std::to_string(containerPos.x);
	if (ImGui::DragFloat("##TX", &containerTranslation.x, 1.0f, 0.0f, 0.0f, formatX.c_str())) {
		container->translates(glm::vec3(containerTranslation.x, 0.0f, 0.0f) * dt);
		containerTranslation.x = 0.0f;
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.0f);
	std::string formatY = "Y: " + std::to_string(containerPos.y);
	if (ImGui::DragFloat("##TY", &containerTranslation.y, 1.0f, 0.0f, 0.0f, formatY.c_str())) {
		container->translates(glm::vec3(0.0f, containerTranslation.y, 0.0f) * dt);
		containerTranslation.y = 0.0f; 
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.0f);
	std::string formatZ = "Z: " + std::to_string(containerPos.z);
	if (ImGui::DragFloat("##TZ", &containerTranslation.z, 1.0f, 0.0f, 0.0f, formatZ.c_str())) {
		container->translates(glm::vec3(0.0f, 0.0f, containerTranslation.z) * dt);
		containerTranslation.z = 0.0f;
	}
	ImGui::EndGroup();

	// container scale
	static glm::vec3 containerScale = glm::vec3(0.0f);
	glm::vec3 currentScale = container->getCurrentScale();
	ImGui::BeginGroup();
	ImGui::Text("Scale"); 
	ImGui::SameLine();
	char scaleFmtX[32], scaleFmtY[32], scaleFmtZ[32];
	std::snprintf(scaleFmtX, sizeof(scaleFmtX), "X: %.2f", currentScale.x);
	std::snprintf(scaleFmtY, sizeof(scaleFmtY), "Y: %.2f", currentScale.y);
	std::snprintf(scaleFmtZ, sizeof(scaleFmtZ), "Z: %.2f", currentScale.z);
	ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragFloat("##SX", &containerScale.x, 1.0f, 0.0f, 0.0f, scaleFmtX)) {
		container->scales(glm::vec3(containerScale.x, 0.0f, 0.0f) * dt);
		containerScale.x = 0.0f;
	}
	ImGui::SameLine(); ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragFloat("##SY", &containerScale.y, 1.0f, 0.0f, 0.0f, scaleFmtY)) {
		container->scales(glm::vec3(0.0f, containerScale.y, 0.0f) * dt);
		containerScale.y = 0.0f;
	}
	ImGui::SameLine(); ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragFloat("##SZ", &containerScale.z, 1.0f, 0.0f, 0.0f, scaleFmtZ)) {
		container->scales(glm::vec3(0.0f, 0.0f, containerScale.z) * dt);
		containerScale.z = 0.0f;
	}
	ImGui::EndGroup();

	// container rotation
	static glm::vec3 containerRotation = glm::vec3(0.0f);
	glm::vec3 currentRot = container->getCurrentRotation();
	ImGui::BeginGroup();
	ImGui::Text("Rotate"); 
	ImGui::SameLine();
	char rotFmtX[32], rotFmtY[32], rotFmtZ[32];
	std::snprintf(rotFmtX, sizeof(rotFmtX), "X: %.1f", currentRot.x);
	std::snprintf(rotFmtY, sizeof(rotFmtY), "Y: %.1f", currentRot.y);
	std::snprintf(rotFmtZ, sizeof(rotFmtZ), "Z: %.1f", currentRot.z);
	ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragFloat("##RX", &containerRotation.x, 1.0f, 0.0f, 0.0f, rotFmtX)) {
		if (std::abs(containerRotation.x) > 0.01f) {
			container->rotates(containerRotation.x, glm::vec3(1.0f, 0.0f, 0.0f) * dt);
		}
		containerRotation.x = 0.0f;
	}
	ImGui::SameLine(); ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragFloat("##RY", &containerRotation.y, 1.0f, 0.0f, 0.0f, rotFmtY)) {
		if (std::abs(containerRotation.y) > 0.01f) {
			container->rotates(containerRotation.y, glm::vec3(0.0f, 1.0f, 0.0f) * dt);
		}
		containerRotation.y = 0.0f;
	}
	ImGui::SameLine(); ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragFloat("##RZ", &containerRotation.z, 1.0f, 0.0f, 0.0f, rotFmtZ)) {
		if (std::abs(containerRotation.z) > 0.01f) {
			container->rotates(containerRotation.z, glm::vec3(0.0f, 0.0f, 1.0f) * dt);
		}
		containerRotation.z = 0.0f;
	}
	ImGui::EndGroup();

	ImGui::Text("Initial parameters");
	static int numOfParticles = (int)simulation->numOfParticles;
	ImGui::SliderInt("Number of particles", &numOfParticles, 0, 100000);
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
		container->reset(simulation->numOfParticles, simulation->particleSpacing);
	}

	static float obstacleRadius = DEFAULT_OBSTACLE_RADIUS;
	//static int currentObstacleIndex = 0;

	if (ImGui::Button("Add Obstacle")) {
		simulation->addObstacle(simulation->getContainer()->getCurrentPosition(), obstacleRadius);
		currentSelectedObstacleIndex = simulation->getObstaclesCount() - 1;
	}
	ImGui::SameLine();
	ImGui::InputFloat("###or", &obstacleRadius);

	static const char* obstacleTags[] = { 
		"Obstacle #1", 
		"Obstacle #2",
		"Obstacle #3",
		"Obstacle #4",
		"Obstacle #5",
		"Obstacle #6",
		"Obstacle #7",
		"Obstacle #8",
		"Obstacle #9",
		"Obstacle #10",
		"Obstacle #11",
		"Obstacle #12",
		"Obstacle #13",
		"Obstacle #14",
		"Obstacle #15",
		"Obstacle #16",
	};

	unsigned int obstacleCount = simulation->getObstaclesCount();
	if (obstacleCount > 0) {
		ImGui::Combo("Select obstacle", &currentSelectedObstacleIndex, obstacleTags, (int)obstacleCount);

		static glm::vec3 translation = glm::vec3(0.0f);
		glm::vec3 currentPos = simulation->getObstaclePosition((unsigned int)currentSelectedObstacleIndex);
		ImGui::BeginGroup();
		ImGui::Text("Translate"); ImGui::SameLine();
		char fmtX[32], fmtY[32], fmtZ[32];
		std::snprintf(fmtX, sizeof(fmtX), "X: %.2f", currentPos.x);
		std::snprintf(fmtY, sizeof(fmtY), "Y: %.2f", currentPos.y);
		std::snprintf(fmtZ, sizeof(fmtZ), "Z: %.2f", currentPos.z);
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::DragFloat("##TX###opX", &translation.x, 1.0f, 0.0f, 0.0f, fmtX)) {
			glm::vec3 pos = simulation->getObstaclePosition((unsigned int)currentSelectedObstacleIndex);
			pos.x += translation.x * dt;
			simulation->setObstaclePosition((unsigned int)currentSelectedObstacleIndex, pos);
			translation.x = 0.0f;
		
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::DragFloat("##TY###opY", &translation.y, 1.0f, 0.0f, 0.0f, fmtY)) {
			glm::vec3 pos = simulation->getObstaclePosition((unsigned int)currentSelectedObstacleIndex);
			pos.y += translation.y * dt;
			simulation->setObstaclePosition((unsigned int)currentSelectedObstacleIndex, pos);
			translation.y = 0.0f;
		}
		ImGui::SameLine(); 
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::DragFloat("##TZ###opZ", &translation.z, 1.0f, 0.0f, 0.0f, fmtZ)) {
			glm::vec3 pos = simulation->getObstaclePosition((unsigned int)currentSelectedObstacleIndex);
			pos.z += translation.z * dt;
			simulation->setObstaclePosition((unsigned int)currentSelectedObstacleIndex, pos);
			translation.z = 0.0f;
		}
		ImGui::EndGroup();

		float radius = simulation->getObstacleRadius((unsigned int)currentSelectedObstacleIndex);
		if (ImGui::SliderFloat("Radius###oor", &radius, 0.5f, 20.0f)) {
			simulation->setObstacleRadius((unsigned int)currentSelectedObstacleIndex, radius);
		}

		if (ImGui::Button("Remove current obstace")) {
			simulation->removeObstacle((unsigned int)currentSelectedObstacleIndex);
			currentSelectedObstacleIndex = 0;
		}
	}

	if (ImGui::Button("Clear obstacles")) {
		currentSelectedObstacleIndex = 0;
		simulation->clearObstacles();
	}


	ImGui::End();
}

void GUIHandler::handleSettingsGUI() {
	FluidRenderer* renderer = engine->getRenderer();
	FluidContainer* container = engine->getSimulation()->getContainer();

	ImGui::Begin("Settings");
	if (ImGui::Button("Toggle fullscreen")) {
		engine->toggleFullScreen();
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

	static float renderDistance = renderer->getCamera()->farPlane;
	if (ImGui::SliderFloat("Render distance", &renderDistance, 5.0f, 1000.0f, "%.1f")) {
		renderer->setRenderDistance(renderDistance);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##rd")) {
		renderer->setRenderDistance(DEFAULT_RENDER_DISTANCE);
		renderDistance = renderer->getCamera()->farPlane;
	}

	static const char* renderingModeTexts[] = { "Basic", "Raymarching" };
	static int selectedRenderingModeIndex = (int)renderer->renderingMode;
	if (ImGui::Combo("Rendering mode", &selectedRenderingModeIndex, renderingModeTexts, IM_ARRAYSIZE(renderingModeTexts))) {
		renderer->renderingMode = (FluidRenderer::RenderingMode)selectedRenderingModeIndex;
	}

	if (selectedRenderingModeIndex == FluidRenderer::RenderingMode::RAYMARCHING) {
		ImGui::Text("Raymarching settings:");

		static int raySteps = (int)renderer->getRaymarcher()->steps;
		if (ImGui::SliderInt("steps", &raySteps, 10, 500)) {
			renderer->getRaymarcher()->steps = (unsigned int)raySteps;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##rms")) {
			renderer->getRaymarcher()->steps = FluidRaymarcherConfig::DEFAULT_STEPS;
			raySteps = (int)renderer->getRaymarcher()->steps;
		}

		glm::vec3 color = renderer->getRaymarcher()->colorAbsorbtionCoefficient;
		if (ImGui::InputFloat3("Color absorbtion coefficent", &color[0])) {
			renderer->getRaymarcher()->colorAbsorbtionCoefficient = color;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##lc")) {
			renderer->getRaymarcher()->colorAbsorbtionCoefficient = FluidRaymarcherConfig::DEFAULT_LIGHT_COLOR;
			color = renderer->getRaymarcher()->colorAbsorbtionCoefficient;
		}

		static float smoothingRadius = renderer->getRaymarcher()->surfaceSmoothingRadius;
		if (ImGui::SliderFloat("Surface smoothing radius", &smoothingRadius, 0.01f, 5.0f, "%.2f")) {
			renderer->getRaymarcher()->surfaceSmoothingRadius = smoothingRadius;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##ssr")) {
			renderer->getRaymarcher()->surfaceSmoothingRadius = FluidRaymarcherConfig::DEFAULT_SURFACE_SMOOTHING_RADIUS;
			smoothingRadius = renderer->getRaymarcher()->surfaceSmoothingRadius;
		}

		static float airInddex = renderer->getRaymarcher()->airRefractionIndex;
		if (ImGui::SliderFloat("Air refraction index", &airInddex, 0.001f, 2.0f, "%.3f")) {
			renderer->getRaymarcher()->airRefractionIndex = airInddex;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##ari")) {
			renderer->getRaymarcher()->airRefractionIndex = FluidRaymarcherConfig::DEFAULT_AIR_REFRACTION_INDEX;
			airInddex = renderer->getRaymarcher()->airRefractionIndex;
		}

		static float fluidInddex = renderer->getRaymarcher()->fluidRefractionIndex;
		if (ImGui::SliderFloat("Fluid refraction index", &fluidInddex, 0.001f, 2.0f, "%.3f")) {
			renderer->getRaymarcher()->fluidRefractionIndex = fluidInddex;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##fri")) {
			renderer->getRaymarcher()->fluidRefractionIndex = FluidRaymarcherConfig::DEFAULT_FLUID_REFRACTION_INDEX;
			fluidInddex = renderer->getRaymarcher()->fluidRefractionIndex;
		}

		static float densityMulitplier = renderer->getRaymarcher()->densityMultiplier;
		if (ImGui::SliderFloat("Density multiplier", &densityMulitplier, 0.001f, 1.0f, "%.3f")) {
			renderer->getRaymarcher()->densityMultiplier = densityMulitplier;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##dm")) {
			renderer->getRaymarcher()->densityMultiplier = FluidRaymarcherConfig::DEFAULT_DENSITY_MULTIPLIER;
			densityMulitplier = renderer->getRaymarcher()->densityMultiplier;
		}

		static float isoLevel = renderer->getRaymarcher()->isoLevel;
		if (ImGui::SliderFloat("Iso level", &isoLevel, 0.01f, 5.0f, "%.2f")){
			renderer->getRaymarcher()->isoLevel = isoLevel;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##il")) {
			renderer->getRaymarcher()->isoLevel = FluidRaymarcherConfig::DEFAULT_ISO_LEVEL;
			isoLevel = renderer->getRaymarcher()->isoLevel;
		}

		static float isoThresholdMultiplier = renderer->getRaymarcher()->isoThresholdMultiplier;
		if (ImGui::SliderFloat("Iso threshold multiplier", &isoThresholdMultiplier, 0.01f, 5.0f, "%.2f")) {
			renderer->getRaymarcher()->isoThresholdMultiplier = isoThresholdMultiplier;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##itm")) {
			renderer->getRaymarcher()->isoThresholdMultiplier = FluidRaymarcherConfig::DEFAULT_ISO_THRESHOLD_MULTIPLIER;
			isoThresholdMultiplier = renderer->getRaymarcher()->isoThresholdMultiplier;
		}

		static int maxBounces = (int)renderer->getRaymarcher()->maxNumOfBounces;
		if (ImGui::SliderInt("Max bounces", &maxBounces, 1, 20)) {
			renderer->getRaymarcher()->maxNumOfBounces = (unsigned int)maxBounces;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##mb")) {
			renderer->getRaymarcher()->maxNumOfBounces = FluidRaymarcherConfig::DEFAULT_MAX_NUM_OF_BOUNCES;
			maxBounces = (int)renderer->getRaymarcher()->maxNumOfBounces;
		}

		if (ImGui::Button("Reload shader")) {
			renderer->getRaymarcher()->reloadShader();
		}
	}

	static bool showEnvMap = renderer->showEnvMap;
	if (ImGui::Checkbox("Show background", &showEnvMap)) {
		renderer->showEnvMap = showEnvMap;
	}

	if (showEnvMap) {
		CubeMapManager* cubeMapManager = renderer->getCubeMapManager();
		static std::vector<std::string> envmapNames = cubeMapManager->getCubeMapTextureNames();
		int currentEnvmapIndex = (int)cubeMapManager->getCurrentCubeMapIndex();
		if (ImGui::Combo("Select background", &currentEnvmapIndex,
			[](void* data, int idx) {
				auto* vectorPtr = static_cast<std::vector<std::string>*>(data);
				return (*vectorPtr)[idx].c_str();
			},
			&envmapNames, (int)envmapNames.size()))
		{
			cubeMapManager->setCubeMapIndex(currentEnvmapIndex);
		}

		if (ImGui::Button("Reload background")) {
			cubeMapManager->reloadCubeMaps();
			envmapNames = cubeMapManager->getCubeMapTextureNames();
		}
	}
	else {
		static glm::vec3 bg = renderer->backgroundColor;
		if (ImGui::InputFloat3("Background color", &bg[0])) {
			renderer->backgroundColor = bg;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##bgc")) {
			renderer->backgroundColor = DEFAULT_BACKGROUND_COLOR;
			bg = renderer->backgroundColor;
		}
	}

	static bool showContainer = renderer->showContainer;
	if (ImGui::Checkbox("Show container", &showContainer)) {
		renderer->showContainer = showContainer;
	}

	if (showContainer) {
		static bool drawAsOutline = renderer->drawContainerAsOutline;
		if (ImGui::Checkbox("Draw as outline", &drawAsOutline)) {
			renderer->drawContainerAsOutline = drawAsOutline;
		}

		if (!drawAsOutline) {
			static float containerOpacity = container->planeOpacity;
			if (ImGui::SliderFloat("Container opacity", &containerOpacity, 0.001f, 1.0f, "%.2f")) {
				container->planeOpacity = containerOpacity;
				container->planeOpacity = containerOpacity;
			}
		}
	}

	static float mouseSensitivity = engine->mouseSensitivity * 100.0f;
	if (ImGui::SliderFloat("Mouse sensitivity", &mouseSensitivity, 0.1f, 25.0f, "%.1f")) {
		engine->mouseSensitivity = mouseSensitivity / 100.0f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##ms")) {
		engine->mouseSensitivity = DEFAULT_MOUSE_SENSITIVITY;
		mouseSensitivity = engine->mouseSensitivity * 100.0f;
	}

	static float mouseScrollSensitivity = engine->mouseScrollSensitivity;
	if (ImGui::SliderFloat("Mouse scroll sensitivity", &mouseScrollSensitivity, 0.1f, 1000.0f, "%.1f")) {
		engine->mouseScrollSensitivity = mouseScrollSensitivity;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##mss")) {
		engine->mouseScrollSensitivity = DEFAULT_MOUSE_SCROLL_SENSITIVITY;
		mouseScrollSensitivity = engine->mouseScrollSensitivity;
	}

	ImGui::End();
}

void GUIHandler::update() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	handleSimulationSettingsGUI();
	handleSettingsGUI();
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

int GUIHandler::getCurrentSelectedObstacleIndex() const {
	return currentSelectedObstacleIndex;
}