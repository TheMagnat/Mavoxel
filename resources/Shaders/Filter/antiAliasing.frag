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

//Not working anymore, it need position but we got velocity
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

    vec4 position = texture(sceneVelocityTexture, TexPos);

    float depth = clamp ( 1.0 - ( (far - position.z) / (far - near) ), 0.0, 1.0);

    float separation = mix(maxSeparation, minSeparation, depth);


    //Findind discountinuties

    float mx = 0.0;

    for (int i = -size; i <= size; ++i) {
        for (int j = -size; j <= size; ++j) {
        // ...

            vec2 newTexCoord = TexPos + (vec2(i, j) * separation) / texSize;

            vec4 positionTemp = texture( sceneVelocityTexture, newTexCoord);

            mx = max(mx, abs(position.z - positionTemp.z));


        }
    }

    float diff = smoothstep(minDistance, maxDistance, mx);

    //vec3 lineColor = texture(sceneTexture, TexPos).rgb * colorModifier;

    return vec4(texture(sceneTexture, TexPos).rgb * (1 - diff), 1.0);
    // return vec4(vec3(1.0 - diff), 1.0);

}




vec3 fxaa() {
    //gl_FragColor.xyz = texture(sceneTexture,texCoords).xyz;
    //return;

    //DEBUG
    vec2 frameBufSize = textureSize(sceneTexture, 0).xy;

    float FXAA_SPAN_MAX = 8.0;
    float FXAA_REDUCE_MUL = 1.0/8.0;
    float FXAA_REDUCE_MIN = 1.0/128.0;

    vec3 rgbNW=texture(sceneTexture,TexPos+(vec2(-1.0,-1.0)/frameBufSize)).xyz;
    vec3 rgbNE=texture(sceneTexture,TexPos+(vec2(1.0,-1.0)/frameBufSize)).xyz;
    vec3 rgbSW=texture(sceneTexture,TexPos+(vec2(-1.0,1.0)/frameBufSize)).xyz;
    vec3 rgbSE=texture(sceneTexture,TexPos+(vec2(1.0,1.0)/frameBufSize)).xyz;
    vec3 rgbM=texture(sceneTexture,TexPos).xyz;

    vec3 luma=vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) / frameBufSize;

    vec3 rgbA = (1.0/2.0) * (
        texture(sceneTexture, TexPos.xy + dir * (1.0/3.0 - 0.5)).xyz +
        texture(sceneTexture, TexPos.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture(sceneTexture, TexPos.xy + dir * (0.0/3.0 - 0.5)).xyz +
        texture(sceneTexture, TexPos.xy + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)){
        return rgbA;
    }else{
        return rgbB;
    }

}




#ifndef FXAA_PRESET
    #define FXAA_PRESET 5
#endif
#if (FXAA_PRESET == 3)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/16.0)
    #define FXAA_SEARCH_STEPS        16
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
#if (FXAA_PRESET == 4)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        24
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
#if (FXAA_PRESET == 5)
    #define FXAA_EDGE_THRESHOLD      (1.0/32.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        32
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif

#define FXAA_SUBPIX_TRIM_SCALE (1.0/(1.0 - FXAA_SUBPIX_TRIM))

// Return the luma, the estimation of luminance from rgb inputs.
// This approximates luma using one FMA instruction,
// skipping normalization and tossing out blue.
// FxaaLuma() will range 0.0 to 2.963210702.
float FxaaLuma(vec3 rgb) {
    return rgb.y * (0.587/0.299) + rgb.x;
}

vec3 FxaaLerp3(vec3 a, vec3 b, float amountOfA) {
    return (vec3(-amountOfA) * b) + ((a * vec3(amountOfA)) + b);
}

vec4 FxaaTexOff(sampler2D tex, vec2 pos, ivec2 off, vec2 rcpFrame) {
    float x = pos.x + float(off.x) * rcpFrame.x;
    float y = pos.y + float(off.y) * rcpFrame.y;
    return texture(tex, vec2(x, y));
}

// pos is the output of FxaaVertexShader interpolated across screen.
// xy -> actual texture position {0.0 to 1.0}
// rcpFrame should be a uniform equal to  {1.0/frameWidth, 1.0/frameHeight}
vec3 FxaaPixelShader(vec2 pos, sampler2D tex, vec2 rcpFrame)
{
    vec3 rgbN = FxaaTexOff(tex, pos.xy, ivec2( 0,-1), rcpFrame).xyz;
    vec3 rgbW = FxaaTexOff(tex, pos.xy, ivec2(-1, 0), rcpFrame).xyz;
    vec3 rgbM = FxaaTexOff(tex, pos.xy, ivec2( 0, 0), rcpFrame).xyz;
    vec3 rgbE = FxaaTexOff(tex, pos.xy, ivec2( 1, 0), rcpFrame).xyz;
    vec3 rgbS = FxaaTexOff(tex, pos.xy, ivec2( 0, 1), rcpFrame).xyz;
    
    float lumaN = FxaaLuma(rgbN);
    float lumaW = FxaaLuma(rgbW);
    float lumaM = FxaaLuma(rgbM);
    float lumaE = FxaaLuma(rgbE);
    float lumaS = FxaaLuma(rgbS);
    float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
    float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
    
    float range = rangeMax - rangeMin;
    if(range < max(FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD))
    {
        return rgbM;
    }
    
    vec3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
    
    float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
    float rangeL = abs(lumaL - lumaM);
    float blendL = max(0.0, (rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE; 
    blendL = min(FXAA_SUBPIX_CAP, blendL);
    
    vec3 rgbNW = FxaaTexOff(tex, pos.xy, ivec2(-1,-1), rcpFrame).xyz;
    vec3 rgbNE = FxaaTexOff(tex, pos.xy, ivec2( 1,-1), rcpFrame).xyz;
    vec3 rgbSW = FxaaTexOff(tex, pos.xy, ivec2(-1, 1), rcpFrame).xyz;
    vec3 rgbSE = FxaaTexOff(tex, pos.xy, ivec2( 1, 1), rcpFrame).xyz;
    rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
    rgbL *= vec3(1.0/9.0);
    
    float lumaNW = FxaaLuma(rgbNW);
    float lumaNE = FxaaLuma(rgbNE);
    float lumaSW = FxaaLuma(rgbSW);
    float lumaSE = FxaaLuma(rgbSE);
    
    float edgeVert = 
        abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
        abs((0.50 * lumaW ) + (-1.0 * lumaM) + (0.50 * lumaE )) +
        abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
    float edgeHorz = 
        abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
        abs((0.50 * lumaN ) + (-1.0 * lumaM) + (0.50 * lumaS )) +
        abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
        
    bool horzSpan = edgeHorz >= edgeVert;
    float lengthSign = horzSpan ? -rcpFrame.y : -rcpFrame.x;
    
    if(!horzSpan)
    {
        lumaN = lumaW;
        lumaS = lumaE;
    }
    
    float gradientN = abs(lumaN - lumaM);
    float gradientS = abs(lumaS - lumaM);
    lumaN = (lumaN + lumaM) * 0.5;
    lumaS = (lumaS + lumaM) * 0.5;
    
    if (gradientN < gradientS)
    {
        lumaN = lumaS;
        lumaN = lumaS;
        gradientN = gradientS;
        lengthSign *= -1.0;
    }
    
    vec2 posN;
    posN.x = pos.x + (horzSpan ? 0.0 : lengthSign * 0.5);
    posN.y = pos.y + (horzSpan ? lengthSign * 0.5 : 0.0);
    
    gradientN *= FXAA_SEARCH_THRESHOLD;
    
    vec2 posP = posN;
    vec2 offNP = horzSpan ? vec2(rcpFrame.x, 0.0) : vec2(0.0, rcpFrame.y); 
    float lumaEndN = lumaN;
    float lumaEndP = lumaN;
    bool doneN = false;
    bool doneP = false;
    posN += offNP * vec2(-1.0, -1.0);
    posP += offNP * vec2( 1.0,  1.0);
    
    for(int i = 0; i < FXAA_SEARCH_STEPS; i++) {
        if(!doneN)
        {
            lumaEndN = FxaaLuma(texture(tex, posN.xy).xyz);
        }
        if(!doneP)
        {
            lumaEndP = FxaaLuma(texture(tex, posP.xy).xyz);
        }
        
        doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
        doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
        
        if(doneN && doneP)
        {
            break;
        }
        if(!doneN)
        {
            posN -= offNP;
        }
        if(!doneP)
        {
            posP += offNP;
        }
    }
    
    float dstN = horzSpan ? pos.x - posN.x : pos.y - posN.y;
    float dstP = horzSpan ? posP.x - pos.x : posP.y - pos.y;
    bool directionN = dstN < dstP;
    lumaEndN = directionN ? lumaEndN : lumaEndP;
    
    if(((lumaM - lumaN) < 0.0) == ((lumaEndN - lumaN) < 0.0))
    {
        lengthSign = 0.0;
    }
 

    float spanLength = (dstP + dstN);
    dstN = directionN ? dstN : dstP;
    float subPixelOffset = (0.5 + (dstN * (-1.0/spanLength))) * lengthSign;
    vec3 rgbF = texture(tex, vec2(
        pos.x + (horzSpan ? 0.0 : subPixelOffset),
        pos.y + (horzSpan ? subPixelOffset : 0.0))).xyz;
        
    return FxaaLerp3(rgbL, rgbF, blendL); 
}

void main() {

    

    // outFragColor = outLining();

    // outFragColor.xyz = fxaa();
    // outFragColor.a = 1.0;

    //ICI
    vec2 frameBufSize = textureSize(sceneTexture, 0).xy;
    vec4 SourceSize = vec4(frameBufSize, 1.0 / frameBufSize); //either TextureSize or InputSize

    outFragColor = vec4(FxaaPixelShader(TexPos, sceneTexture, vec2(SourceSize.z, SourceSize.w)), 1.0) * 1.0;
    outVelocity = texture(sceneVelocityTexture, TexPos);
    //A ICI

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
