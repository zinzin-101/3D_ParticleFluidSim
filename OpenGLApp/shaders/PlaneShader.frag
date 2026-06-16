#version 440 core

out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 color;
uniform float opacity;

void main() {
    FragColor = vec4(color, opacity);
}
