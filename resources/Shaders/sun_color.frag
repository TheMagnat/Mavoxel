#version 450 core

layout (location = 0) out vec4 outFragColor;

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexPos;

void main() {
    outFragColor = vec4(1.0);
}
