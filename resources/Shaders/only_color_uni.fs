#version 330 core

out vec4 outFragColor;

in vec3 FragPos;

uniform vec3 Color;

void main() {
    outFragColor = vec4(Color, 1.0);
}
