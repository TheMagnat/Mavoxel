#version 450 core
#extension GL_GOOGLE_include_directive : require

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outLightColor;
layout (location = 2) out vec4 outVelocity;

layout (location = 0) in vec2 TexPos;

#include "filterUniforms.frag"

//Full rendered scene texture
layout(binding = 2) uniform sampler2D sceneTexture;
layout(binding = 3) uniform sampler2D sceneLightTexture;
layout(binding = 4) uniform sampler2D sceneVelocityTexture;

layout(binding = 5) uniform sampler2D lastFramePositionTexture;


struct MotionBlurResult {
    vec4 color;
    vec4 lightColor;
};

MotionBlurResult motionBlur() {

    int   size       = 30;
    float separation = 0.4;

    MotionBlurResult result;

    result.color = texture(sceneTexture, TexPos);
    result.lightColor = texture(sceneLightTexture, TexPos);
    
    vec2 direction = texture(sceneVelocityTexture, TexPos).xy;
    if (length(direction) == 0.0) return result;


    //TODO: Add a delta ?
    if (length(direction) <= 0.0) return result;

    direction.xy *= separation;

    vec2  forward  = TexPos;
    vec2  backward = TexPos;

    vec2 directionStep = direction / size;

    for (int i = 0; i < size; ++i) {
        
        backward  += directionStep; //From new to last
        forward -= directionStep; //From new to theoretical next

        result.color += texture(sceneTexture, backward);
        result.color += texture(sceneTexture, forward);

        result.lightColor += texture(sceneLightTexture, backward);
        result.lightColor += texture(sceneLightTexture, forward);

    }

    result.color /= size * 2 + 1;
    result.lightColor /= size * 2 + 1;

    // result.color = result.color * 0.5 + 0.5 * texture(sceneTexture, TexPos);
    // result.lightColor = result.lightColor * 0.5 + 0.5 * texture(sceneLightTexture, TexPos);

    return result;

}

void main() {

    MotionBlurResult motionBlurResult = motionBlur();

    outFragColor = vec4(vec3(motionBlurResult.color), 1);
    outLightColor = vec4(vec3(motionBlurResult.lightColor), 1);
    outVelocity = texture(sceneVelocityTexture, TexPos);

    // bool debug = true;
    if (debug == 1) {
        
        float middleSize = 0.001;

        if (TexPos.x + middleSize > 0.5 && TexPos.x - middleSize < 0.5) {
            outFragColor = vec4(vec3(1.0), 1.0);
        }
        else if (TexPos.x > 0.5) {
            outFragColor = vec4(vec3(texture(sceneTexture, TexPos)), 1);
        }
    
    }

    // outFragColor = vec4(vec3(texture(sceneTexture, TexPos)), 1);
}
