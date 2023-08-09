#version 450 core

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 1) uniform colorInformation {
    vec3 Color;
};

void main() {
    outFragColor = vec4(Color, 1.0);
}
