#pragma once
#include <glm/glm.hpp>

struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	Particle();
	glm::vec3 getPosition() const;
	glm::vec3 getVelocity() const;
};