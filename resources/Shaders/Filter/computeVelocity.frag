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
    outFragColor = vec4(vec3(texture(sceneTexture, TexPos)), 1);
    outLightColor = vec4(vec3(texture(sceneLightTexture, TexPos)), 1);

}
