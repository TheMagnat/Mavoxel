#version 450 core
#extension GL_GOOGLE_include_directive : require

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outLightColor;
layout (location = 2) out vec4 outPosition;

layout (location = 0) in vec2 TexPos;

#include "filterUniforms.frag"

//Full rendered scene texture
layout(binding = 2) uniform sampler2D sceneTexture;
layout(binding = 3) uniform sampler2D sceneLightTexture;
layout(binding = 4) uniform sampler2D scenePositionTexture;


vec2 lightPositionOnScreen[1] = vec2[1](sun);
int n = 1;

vec3 ray2() {

    float decay=0.984815;
    float exposure=0.21;
    float density=1.2;
    float weight=0.48767;

    /// NUM_SAMPLES will describe the rays quality, you can play with
    int NUM_SAMPLES = 100;
    
    vec3 totalColor = vec3(0);

    for (int clpos=0; clpos < n; clpos++)
    {
        vec2 center = lightPositionOnScreen[clpos].xy - vec2(0.5);
        float lenn = length(center) * 0.45;
        

        float factor = 1.0 - clamp(lenn, 0.0, 1.0);
        if (factor == 0.0) continue;

        vec2 tc = TexPos.xy;
        vec2 deltaTexCoord = (tc - lightPositionOnScreen[clpos].xy);
        deltaTexCoord *= 1.0 / (float(NUM_SAMPLES) * density);
        float illuminationDecay = 1.0;

        vec3 color = texture(sceneLightTexture, tc.xy).rgb * 0.1;

        for(int i=0; i < NUM_SAMPLES ; i++)
        {
            tc -= deltaTexCoord;
            vec3 value = texture(sceneLightTexture, tc).rgb * 0.1;

            //if (length(value) <= 0.001) continue;

            value *= illuminationDecay * weight;
            color += value;
            illuminationDecay *= decay;
        }

        totalColor += color * factor;
        

    }
 
    return totalColor * exposure;
    // vec4 realColor = texture(sceneLightTexture, TexPos.xy);
    // return ((vec4((vec3(totalColor.r,totalColor.g,totalColor.b) * exposure), 1)) + (realColor*(1.1)));


}


vec4 ray(){

    vec4 tempoFragColor = vec4(0, 0, 0, 1);

    int nbDecalage = 150;


    //Vec2(0.5) represente le centre de la lumière
    vec2 delta = vec2(TexPos - vec2(0.5));
    vec2 textCoodsSave = TexPos;
    delta *= 1.0 / float(nbDecalage);
    
    
    for(int i=0; i < nbDecalage; ++i){

            textCoodsSave -= delta;
            vec4 pixelDecale = texture(sceneLightTexture, textCoodsSave);
            
            pixelDecale *= 0.005;
            
            tempoFragColor += pixelDecale;
            
    }
    
    return tempoFragColor;
}


vec4 outLining() {

    float minSeparation = 1.0;
    float maxSeparation = 1.0;
    float minDistance   = 0.25;
    float maxDistance   = 0.65;
    int   size          = 2;
    vec3 colorModifier  = vec3(0.324, 0.063, 0.099);

    //TEMPO:
    float near = 0;
    float far = 100;

    vec2 texSize = textureSize(sceneTexture, 0).xy;

    vec4 position = newProjectionViewMat * texture(scenePositionTexture, TexPos);

    float depth = clamp ( 1.0 - ( (far - position.z) / (far - near) ), 0.0, 1.0);

    float separation = mix(maxSeparation, minSeparation, depth);


    //Findind discountinuties

    float mx = 0.0;

    for (int i = -size; i <= size; ++i) {
        for (int j = -size; j <= size; ++j) {
        // ...

            vec2 newTexCoord = TexPos + (vec2(i, j) * separation) / texSize;

            vec4 positionTemp = newProjectionViewMat * texture( scenePositionTexture, newTexCoord);

            mx = max(mx, abs(position.z - positionTemp.z));


        }
    }

    float diff = smoothstep(minDistance, maxDistance, mx);

    //vec3 lineColor = texture(sceneTexture, TexPos).rgb * colorModifier;

    return vec4(texture(sceneTexture, TexPos).rgb * (1 - diff), 1.0);
    // return vec4(vec3(1.0 - diff), 1.0);

}

void main() {

    
    
    vec3 color = texture(sceneTexture, TexPos).rgb;
    vec3 raycolor = ray2();

    vec3 finalColor = color + 1.15*raycolor;

    outFragColor = vec4(finalColor, 1);
    outLightColor = texture(sceneLightTexture, TexPos);
    outPosition = texture(scenePositionTexture, TexPos);



    // outFragColor = outLining();

    // outFragColor.xyz = fxaa();
    // outFragColor.a = 1.0;

    // vec2 frameBufSize = textureSize(sceneTexture, 0).xy;
    // vec4 SourceSize = vec4(frameBufSize, 1.0 / frameBufSize); //either TextureSize or InputSize

    // outFragColor = vec4(FxaaPixelShader(TexPos, sceneTexture, vec2(SourceSize.z, SourceSize.w)), 1.0) * 1.0;


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
