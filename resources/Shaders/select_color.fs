#version 330 core

out vec4 outFragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexPos;

uniform vec3 viewPos;
uniform float time;

void main() {

    float scaledTime = time * 3;

    //Value between 0 and 1
    float distFromEdge = max(abs(TexPos.x - 0.5), abs(TexPos.y - 0.5)) * 2;
    //float distFromCenter = length(TexPos - 0.5);


    if (distFromEdge > (0.925 + cos(scaledTime)*0.035)) outFragColor = vec4(1.0, 1.0, 1.0, 0.75);
    //if (distFromCenter > (0.6 + cos(scaledTime)*0.025)) outFragColor = vec4(1.0, 1.0, 1.0, 0.75);
    else discard;
}
