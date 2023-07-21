#version 450 core

layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec2 TexPos;

//Full rendered scene texture
layout(binding = 0) uniform sampler2D sceneTexture;


vec2 lightPositionOnScreen[1] = vec2[1](vec2(0.5, 0.5));
int n = 1;

vec4 ray2() {

    float decay=0.97815;
    float exposure=0.21;
    float density=0.926;
    float weight=0.58767;

    /// NUM_SAMPLES will describe the rays quality, you can play with
    int NUM_SAMPLES = 150;
    
    vec4 totalColor = vec4(0);

    for (int clpos=0; clpos < n; clpos++)
        {
        vec2 tc = TexPos.xy;
        vec2 deltaTexCoord = (tc - lightPositionOnScreen[clpos].xy);
        deltaTexCoord *= 1.0 / float(NUM_SAMPLES) * density;
        float illuminationDecay = 1.0;

        vec4 color = texture(sceneTexture, tc.xy) * 0.1;

        for(int i=0; i < NUM_SAMPLES ; i++)
        {
            tc -= deltaTexCoord;
            vec4 value = texture(sceneTexture, tc) * 0.1;
            value *= illuminationDecay * weight;
            color += value;
            illuminationDecay *= decay;
        }

        totalColor += color;
        

    }
 
    // return color;
    vec4 realColor = texture(sceneTexture, TexPos.xy);
    return ((vec4((vec3(totalColor.r,totalColor.g,totalColor.b) * exposure), 1)) + (realColor*(1.1)));


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
            vec4 pixelDecale = texture(sceneTexture, textCoodsSave);
            
            pixelDecale *= 0.005;
            
            tempoFragColor += pixelDecale;
            
    }
    
    return tempoFragColor;
}

void main() {
    
    vec4 color = texture(sceneTexture, TexPos);
    vec4 raycolor = ray2();

    vec4 finalColor = 0.70*color + 1.15*raycolor;

    outFragColor = raycolor;

    // outFragColor = texture(sceneTexture, TexPos);
}
