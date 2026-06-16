#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTransform;

uniform mat3 normalMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform float radius;
uniform float renderScale;

out vec3 FragPos;
out vec3 Normal;

void main() {
    vec3 worldPos = aTransform.xyz;

    vec3 pos = ((radius * aPos) + worldPos) * renderScale;

    FragPos = pos;
    Normal = normalMatrix * aNormal;
    gl_Position = projection * view * vec4(pos, 1.0);
}