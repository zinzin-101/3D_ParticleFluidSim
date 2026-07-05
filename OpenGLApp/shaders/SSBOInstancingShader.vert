#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

layout(std430, binding = 0) buffer ParticlePositions { float positions[]; };
layout(std430, binding = 1) buffer ParticleVelocities { float velocities[]; };

uniform mat3 normalMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform float radius;
uniform float renderScale;

out vec3 FragPos;
out vec3 Normal;
out float Speed;

vec3 getPositionVec3(uint id) {
    uint base = id * 3;
    return vec3(positions[base], positions[base+1], positions[base+2]);
}

vec3 getVelocityVec3(uint id) {
    uint base = id * 3;
    return vec3(velocities[base], velocities[base+1], velocities[base+2]);
}

void main() {
    uint id = uint(gl_InstanceID);

    vec3 worldPos = getPositionVec3(id);

    vec3 pos = ((radius * aPos) + worldPos) * renderScale;

    FragPos = pos;
    Normal = normalMatrix * aNormal;
    Speed = length(getVelocityVec3(id));
    gl_Position = projection * view * vec4(pos, 1.0);
}