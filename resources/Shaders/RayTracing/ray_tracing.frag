#version 450 core
#extension GL_GOOGLE_include_directive : require

// Global
#define PI 3.1415926538

#include "inputs.frag"

//POSITION: DEBUG
//Inpute / Outputs
layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outLightColor;
layout (location = 2) out vec4 outPosition;

layout (location = 0) in vec2 TexPos;

#include "rayTracing.frag"

//PRAMS
#define RAY_DISTANCE 384

vec3 createRay(vec2 px, mat4 PInv, mat4 VInv) {
  
    // convert pixel to NDS
    // [0,1] -> [-1,1]
    px.y = 1 - px.y;
    vec2 pxNDS = px*2. - 1.;
    
    // choose an arbitrary point in the viewing volume
    // z = -1 equals a point on the near plane, i.e. the screen
    vec3 pointNDS = vec3(pxNDS, -1.);

    // as this is in homogenous space, add the last homogenous coordinate
    vec4 pointNDSH = vec4(pointNDS, 1.0);
    // transform by inverse projection to get the point in view space
    vec4 dirEye = PInv * pointNDSH;

    // since the camera is at the origin in view space by definition,
    // the current point is already the correct direction 
    // (dir(0,P) = P - 0 = P as a direction, an infinite point,
    // the homogenous component becomes 0 the scaling done by the 
    // w-division is not of interest, as the direction in xyz will 
    // stay the same and we can just normalize it later
    dirEye.w = 0.;

    // compute world ray direction by multiplying the inverse view matrix
    vec3 dirWorld = (VInv * dirEye).xyz;

    // now normalize direction
    return normalize(dirWorld); 
}

//Old functions...
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
//End old functions

//Main function
void main() {

    vec3 rayDirection = createRay(TexPos, inverse(projection), inverse(view));
    // vec3 rayDirection = getRayDir(rayTarget);

    RayTracingResult rayTracingResult = applyRayTracing(camera.position, rayDirection, RAY_DISTANCE);
    outFragColor = vec4(rayTracingResult.color, 1.0);
    outLightColor = vec4(rayTracingResult.lightColor, 1.0);
    outPosition = rayTracingResult.position;

}
