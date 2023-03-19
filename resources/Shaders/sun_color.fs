#version 330 core

out vec4 outFragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexPos;

uniform vec3 viewPos;

void main() {
    outFragColor = vec4(1.0);
}
