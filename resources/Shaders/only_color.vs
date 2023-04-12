#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Color;

void main(){
    
    vec4 positionToCam = view * vec4(aPos, 1.0);
    gl_Position = projection * positionToCam;
        
    FragPos = aPos;

    Color = aColor;
    
}

