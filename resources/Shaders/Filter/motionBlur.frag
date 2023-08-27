#version 450 core

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outLightColor;
layout (location = 2) out vec4 outPosition;

layout (location = 0) in vec2 TexPos;


//Full rendered scene texture
layout(binding = 0) uniform sampler2D sceneTexture;
layout(binding = 1) uniform sampler2D sceneLightTexture;
layout(binding = 2) uniform sampler2D scenePositionTexture;

layout (set = 0, binding = 4) uniform FilterInformations {
    mat4 oldProjectionViewMat;
    mat4 newProjectionViewMat;
    mat4 view;
};

struct MotionBlurResult {
    vec4 color;
    vec4 lightColor;
};

MotionBlurResult motionBlur() {

    int   size       = 25;
    float separation = 0.6;

    MotionBlurResult result;

    result.color = texture(sceneTexture, TexPos);
    result.lightColor = texture(sceneLightTexture, TexPos);
    
    vec4 position1 = texture(scenePositionTexture, TexPos);
    if (position1.z == 0.0) return result;

    vec4 position0 = newProjectionViewMat * inverse(oldProjectionViewMat) * position1;

    position0      = view * position0;
    position0.xyz /= position0.w;
    position0.xy   = position0.xy * 0.5 + 0.5;

    position1      = view * position1;
    position1.xyz /= position1.w;
    position1.xy   = position1.xy * 0.5 + 0.5;

    vec2 direction = position0.xy - position1.xy;

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
    outPosition = texture(scenePositionTexture, TexPos);

    // outFragColor = vec4(vec3(texture(sceneTexture, TexPos)), 1);
    // outLightColor = vec4(vec3(texture(sceneLightTexture, TexPos)), 1);

    // bool debug = true;
    // if (debug) {
        
    //     float middleSize = 0.001;

    //     if (TexPos.x + middleSize > 0.5 && TexPos.x - middleSize < 0.5) {
    //         outFragColor = vec4(vec3(1.0), 1.0);
    //     }
    //     else if (TexPos.x > 0.5) {
    //         outFragColor = vec4(vec3(texture(sceneTexture, TexPos)), 1);
    //     }
    
    // }

    // outFragColor = vec4(vec3(texture(scenePositionTexture, TexPos)), 1);
}
