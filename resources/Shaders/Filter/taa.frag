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

layout(binding = 5) uniform sampler2D lastFrameTexture;
layout(binding = 6) uniform sampler2D lastFrameVelocityTexture;




// Store weight in w component
// vec4 AdjustHDRColor(vec4 color)
// {
//     if(InverseLuminance)
//     {
//         float luminance = dot(color, vec3(0.299, 0.587, 0.114));
//         float luminanceWeight = 1.0 / (1.0 + luminance);
//         return vec4(color, 1.0) * luminanceWeight;
//     }
//     else if(Log)
//     {
//         return vec4(x > 0.0 ? log(x) : -10.0, 1.0); // Guard against nan
//     }
// }

bool InverseLuminance = true;
bool Log = false;

vec4 AdjustHDRColor(vec3 color)
{
    if(InverseLuminance)
    {
        float luminance = dot(color, vec3(0.299, 0.587, 0.114));
        float luminanceWeight = 1.0 / (1.0 + luminance);
        return vec4(color, 1.0) * luminanceWeight;
    }
    else if(Log)
    {
        vec3 test = vec3(bool(color.r > 0.0) ? log(color) : vec3(-10.0));
        return vec4(test, 1.0); // Guard against nan
    }
}

vec4 taa() {

    vec2 uv = TexPos;
    vec2 velocity = texture(sceneVelocityTexture, TexPos).xy;
    // return vec4(velocity, 0.0, 1.0);
    vec2 sourceDimensions = textureSize(sceneTexture, 0);

    // velocity = vec2(0.0);

    vec2 reprojectedUV = velocity + TexPos;

    vec4 currentColor = texture(sceneTexture, TexPos);
    vec4 previousColor = texture(lastFrameTexture, reprojectedUV);

    // Arbitrary out of range numbers
    vec3 minColor = vec3(9999.0), maxColor = vec3(-9999.0);

    // Sample a 3x3 neighborhood to create a box in color space
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            vec3 color = texture(sceneTexture, uv + (vec2(x, y) / sourceDimensions)).rgb; // Sample neighbor

            // color = AdjustHDRColor(color).rgb;

            minColor = min(minColor, color); // Take min and max
            maxColor = max(maxColor, color);

        }
    }


    //NEW ADAPT
    // currentColor = AdjustHDRColor(currentColor.rgb);
    // previousColor = AdjustHDRColor(previousColor.rgb);
    //NEW END

    // Clamp previous color to min/max bounding box
    vec3 previousColorClamped = clamp(previousColor.rgb, minColor, maxColor);

    
    //// Velocity Rejection...
    // Assume we store UV offsets
    // vec2 currentVelocityUV = velocity;

    // // Read previous velocity
    // vec2 previousVelocityUV = texture(lastFrameVelocityTexture, uv + currentVelocityUV).xy;

    // // Compute length between vectors
    // float velocityLength = length(previousVelocityUV - currentVelocityUV);

    // // Adjust value
    // float velocityDisocclusion = clamp((velocityLength - 0.001) * 10.0, 0.0, 1.0);

    // // Calculate base accumulated quantity
    // vec3 accumulation = currentColor * 0.1 + previousColorClamped * 0.9;
    // // Blend

    // // Lerp towards a backup value - could be a blurred version derived from the neighborhood
    // vec3 outputColor = lerp(accumulation, currentFrameBlurred, velocityDisocclusion);
    //// End velocity Rejection...


    ///// ICI REPRENDRE

    // float currentWeight = 0.1 * currentColor.a;
    // float previousWeight = 0.9 * previousColor.a;

    // vec3 outputColor = (currentColor.rgb * currentWeight + previousColorClamped.rgb * previousWeight);

    // outputColor /= (currentWeight + previousWeight); // Normalize back. Note that this has no effect in the log case

    // if(Log)
    // {
    //     outputColor = exp(outputColor); // Undo log transformation
    // }
    //END NEW

    float testValue = length(velocity) * 100;
    testValue = clamp(testValue, 0.0, 1.0) * 0.9;
    // testValue = 0.0;
    
    vec3 outputColor = currentColor.rgb * (0.1 + testValue) + previousColorClamped.rgb * (0.9 - testValue);

    return vec4(outputColor, 1.0);
}

void main() {
    vec4 currentFrame = vec4(vec3(texture(sceneTexture, TexPos)), 1);
    vec4 bourre = vec4(vec3(texture(lastFrameTexture, TexPos).xyz), 1); //lastFrameColor

    // outFragColor = currentFrame * 0.75 + bourre * 0.25;
    // outFragColor = currentFrame;

    // outFragColor = vec4(taa(), 1);
    outFragColor = taa();
    outVelocity = texture(sceneVelocityTexture, TexPos);

    // outFragColor = vec4(vec3(texture(lastFrameVelocityTexture, TexPos).xyz * 20), 1); //lastFrameColor
    

    // bool debug = true;
    if (debug == 1) {
        
        float middleSize = 0.001;

        if (TexPos.x + middleSize > 0.5 && TexPos.x - middleSize < 0.5) {
            outFragColor = vec4(vec3(1.0), 1.0);
        }
        else if (TexPos.x > 0.5) {
            outFragColor = vec4(vec3(texture(sceneTexture, TexPos).xyz), 1);
        }
    
    }
    // outFragColor = vec4(vec3(texture(sceneTexture, TexPos).xyz), 1);
}
