#version 450 core
#extension GL_GOOGLE_include_directive : require

//POSITION: DEBUG
//Inpute / Outputs
layout (location = 0) out vec4 outFragColor;
layout (location = 0) in vec2 TexPos;

#include "raycastLight.frag"

//PRAMS
#define RAY_DISTANCE 384


vec3 getRayTarget() {
    vec2 uv = TexPos * 2 - 1.0;
    uv.x *= xRatio;
    //vec2 ndc = (2.0 * TexPos - screenSize) / screenSize;
    //vec2 ndc = (2.0 * TexPos - screenSize) / screenSize;
    vec3 near = camera.position + camera.front * 2.5;
    vec3 right = camera.right * uv.x;
    vec3 up = -camera.up * uv.y;
    return near + right + up;
}

vec3 getRayDir(vec3 rayTarget) {
    return normalize(rayTarget - camera.position);
}


//Main function
void main() {

    vec3 rayTarget = getRayTarget();
    vec3 rayDirection = getRayDir(rayTarget);

    //HERE TODO ADAPT TO OCTREE
    vec3 colorResult = castRay(camera.position, rayDirection, RAY_DISTANCE);
    outFragColor = vec4(colorResult, 1.0);

}
