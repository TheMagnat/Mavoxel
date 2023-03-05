#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in float aId;

uniform mat3 modelNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out float Id;


void main(){
    
    vec4 positionToCam = view * model * vec4(aPos, 1.0);
    gl_Position = projection * positionToCam;
        
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = modelNormal * aNormal;

    Id = aId;
    
}

