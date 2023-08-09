#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;
layout (location = 3) in float aAO;
layout (location = 4) in float aId;



layout(set = 0, binding = 0) uniform ModelViewProjection {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 modelNormal;
} mvp;

layout(location = 0) out vec3 FragPos;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec2 TexPos;
layout(location = 3) out float AO;
layout(location = 4) out float Id;


void main(){
    
    vec4 positionToCam = mvp.view * mvp.model * vec4(aPos, 1.0);
    gl_Position = mvp.projection * positionToCam;
        
    FragPos = vec3(mvp.model * vec4(aPos, 1.0));
    Normal = vec3(mvp.modelNormal * vec4(aNormal, 1.0));

    TexPos = aTex;

    Id = aId;
    AO = aAO;
    
}

