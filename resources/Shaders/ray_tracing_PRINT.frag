#version 450 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : enable

//POSITION: DEBUG
//Inpute / Outputs
layout (location = 0) out vec4 outFragColor;
layout (location = 0) in vec2 TexPos;

//Structures definition
struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
};

struct Sun {
    vec3 position;
};

//Uniforms
layout (set = 0, binding = 0) uniform RayCastInformations {
    float xRatio;
    Camera camera;
    Sun sun;
    float time;
};


int id = int(TexPos.x * 1584541.0 + TexPos.y * 51471445.0 * 0.5 * 1.9);
#define CENTER (id == 0 && (mod(time, 1.0) < 0.1))
#define CENTER_NO_TIME (id == 0)

#include "raycastLight.frag"

//PRAMS
#define RAY_DISTANCE 512




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

    if (CENTER){
        debugPrintfEXT("\n\n\nShader Start !!! \n");
        debugPrintfEXT("Cast ray\n");
    }
    //HERE TODO ADAPT TO OCTREE
    vec3 colorResult = castRay(camera.position, rayDirection, RAY_DISTANCE);
    outFragColor = vec4(colorResult, 1.0);

    bool isFalse = colorResult.g < 0.9;
    vec3 debugColor = vec3(1.0, 0.0, 0.0);
    if (isFalse) {
        debugColor = vec3(0.0, 0.0, 1.0);
    }

    // if(TexPos.x > 0.495 && TexPos.x < 0.505) {
    //     outFragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // }

    vec2 debugTarget = vec2(0.40, 0.40);
    float targetLen = 0.001;

    vec2 debugXLimit = vec2(debugTarget.x - 0.2, debugTarget.x + 0.2);
    vec2 debugYLimit = vec2(debugTarget.y - 0.2, debugTarget.y + 0.2);

    if (TexPos.x > (debugTarget.x - targetLen) && TexPos.x < (debugTarget.x + targetLen) && (TexPos.y < debugYLimit.x || TexPos.y > debugYLimit.y)){
        vec3 resColor = colorResult * 0.5 + debugColor * 0.5;
        outFragColor = vec4(resColor, 1.0);
    }
    if (TexPos.y > (debugTarget.x - targetLen) && TexPos.y < (debugTarget.x + targetLen) && (TexPos.x < debugXLimit.x || TexPos.x > debugXLimit.y)){
        vec3 resColor = colorResult * 0.5 + debugColor * 0.5;
        outFragColor = vec4(resColor, 1.0);
    }

    if (CENTER_NO_TIME) {
        vec3 resColor = colorResult * 0.5 + debugColor * 0.5;
        outFragColor = vec4(resColor, 1.0);
    }

    if (CENTER) {
        debugPrintfEXT("Shader end...\n");
    }

    if (CENTER && isFalse) {
        debugPrintfEXT("@@@@@@@@@@@@@@@@@@@@@@@@@@@ BUG @@@@@@@@@@@@@@@@@@@@@@@@@\n@@@@@@@@@@@@@@@@@@@@@@@@@ BUG @@@@@@@@@@@@@@@@@@@@@@@\n@@@@@@@@@@@@@@@@@@@@@@@@@@@ BUG @@@@@@@@@@@@@@@@@@@@@@@\n");
    }

}
