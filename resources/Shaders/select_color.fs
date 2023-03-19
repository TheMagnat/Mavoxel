#version 330 core

out vec4 outFragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexPos;

uniform vec3 viewPos;

void main() {

    //Value between 0 and 1
    float distFromEdge = max(abs(TexPos.x - 0.5), abs(TexPos.y - 0.5)) * 2;
    //float distFromCenter = length(TexPos - 0.5);


    if (distFromEdge > 0.95) outFragColor = vec4(1.0, 1.0, 1.0, 1.0);
    else discard;
}
