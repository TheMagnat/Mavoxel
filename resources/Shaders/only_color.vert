#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

layout(set = 0, binding = 0) uniform ViewProjection {
    mat4 view;
    mat4 projection;
} vp;

layout (location = 0) out vec3 FragPos;
layout (location = 1) out vec3 Color;

void main(){
    
    vec4 positionToCam = vp.view * vec4(aPos, 1.0);
    gl_Position = vp.projection * positionToCam;
        
    FragPos = aPos;

    Color = aColor;
    
}

