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
layout(binding = 4) uniform sampler2D scenePositionTexture;

layout(binding = 5) uniform sampler2D lastFramePositionTexture;


struct MotionBlurResult {
    vec4 color;
    vec4 lightColor;
};

MotionBlurResult motionBlur() {

    int   size       = 15;
    float separation = 0.6;

    MotionBlurResult result;

    result.color = texture(sceneTexture, TexPos);
    result.lightColor = texture(sceneLightTexture, TexPos);
    
    vec4 currentWorldPosition = texture(scenePositionTexture, TexPos);

    vec2 direction = outVelocity.xy;
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


vec2 getVelocity_old(vec2 uv) {

    //Velocity
    vec4 currentWorldPosition = texture(scenePositionTexture, uv);
    vec4 previousWorldPosition = texture(lastFramePositionTexture, uv);
    if (currentWorldPosition.w == 0) return vec2(0.0);

    vec4 currentPos = newProjectionViewMat * currentWorldPosition;

    //TODO: Utiliser previousWorldPosition !!!
    vec4 previousPos = oldProjectionViewMat * previousWorldPosition;

    // previousPos      = view * previousPos;
    previousPos.xyz /= previousPos.w;
    previousPos.xy   = previousPos.xy * 0.5 + 0.5;

    // currentPos      = view * currentPos;
    currentPos.xyz /= currentPos.w;
    currentPos.xy   = currentPos.xy * 0.5 + 0.5;

    return currentPos.xy - previousPos.xy;
}

vec2 getVelocity(vec2 uv) {

    vec2 sourceDimensions = textureSize(scenePositionTexture, 0).xy;

    //Velocity
    vec4 currentWorldPosition = texture(scenePositionTexture, uv);
    // vec4 previousWorldPosition = texture(lastFramePositionTexture, uv);
    if (currentWorldPosition.w == 0) return vec2(0.0);

    vec4 currentPos = newProjectionViewMat * currentWorldPosition;
    vec4 previousPos = oldProjectionViewMat * currentWorldPosition;

    vec3 currentPosNDC = currentPos.xyz;// / currentPos.w;
    vec3 previousPosNDC = previousPos.xyz; // / previousPos.w;

    //TODO: trouver la bonne formule ?
    return (previousPosNDC.xy - currentPosNDC.xy) / currentPos.w;


}

void main() {

    // vec2 velo_old = getVelocity_old(TexPos);
    vec2 velo = getVelocity(TexPos);

    outVelocity = vec4(velo, 0.0, 0.0);

    MotionBlurResult motionBlurResult = motionBlur();

    // outFragColor = vec4(vec3(motionBlurResult.color), 1);
    // outLightColor = vec4(vec3(motionBlurResult.lightColor), 1);

    outFragColor = vec4(vec3(texture(sceneTexture, TexPos)), 1);
    outLightColor = vec4(vec3(texture(sceneLightTexture, TexPos)), 1);

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
