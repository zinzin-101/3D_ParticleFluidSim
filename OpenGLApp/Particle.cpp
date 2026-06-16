#include "Particle.h"

Particle::Particle() : position(0.0f), velocity(0.0f) {}
glm::vec3 Particle::getPosition() const {
	return position;
}
glm::vec3 Particle::getVelocity() const {
	return velocity;
}