#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

uniform mat3 modelNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPosG;
out vec3 NormalG;
out vec2 texPosG;


void main(){
    
    
    vec4 positionToCam = view * model * vec4(aPos, 1.0);
    
    gl_Position = projection * positionToCam;
        
    FragPosG = vec3(model * vec4(aPos, 1.0));
    
    NormalG = modelNormal * aNormal;
    
    texPosG = aTex;
    
}

