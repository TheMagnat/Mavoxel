#version 450 core

layout (location = 0) in vec3 aPos;

layout(set = 0, binding = 0) uniform ModelViewProjectionNoNormal {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main(){

    vec4 positionToCam = mvp.view * mvp.model * vec4(aPos, 1.0);
    gl_Position = mvp.projection * positionToCam;
            
}

