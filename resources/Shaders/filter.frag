#version 450 core

layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec2 TexPos;

//Full rendered scene texture
layout(binding = 0) uniform sampler2D sceneTexture;
layout(binding = 1) uniform sampler2D sceneLightTexture;

//Uniforms
layout (set = 0, binding = 2) uniform RayCastInformations {
    vec2 sun;
};

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

void main() {
    
    vec3 color = texture(sceneTexture, TexPos).rgb;
    vec3 raycolor = ray2();

    vec3 finalColor = color + 1.15*raycolor;

    outFragColor = vec4(finalColor, 1);

    // outFragColor = raycolor + texture(sceneTexture, TexPos);
}
