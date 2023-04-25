#version 330 core

out vec4 outFragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexPos;
in float AO;
in float Id;

uniform vec3 viewPos;
//uniform Material material;
uniform Light light;
uniform float time;

Material materials[] = Material[](
    //0 - Default / Error
    Material(
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //1 - Grass
    Material(
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //2 - Dirt
    Material(
        vec3(0.819f, 0.466f, 0.263f),
        vec3(0.819f, 0.466f, 0.263f),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //3 - Stone
    Material(
        vec3(0.7176f, 0.6901f, 0.6117f),
        vec3(0.7176f, 0.6901f, 0.6117f),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    )
);

//--------------------------------------------------------------------
// Noise Functions
//--------------------------------------------------------------------

// Modified hash33 by Dave_Hoskins (original does not play well with simplex)
// Original Source: https://www.shadertoy.com/view/4djSRW
vec3 hash33(vec3 p3)
{
	p3 = fract(p3 * vec3(0.1031, 0.11369, 0.13787));
    p3 += dot(p3, p3.yxz + 19.19);
    return -1.0 + 2.0 * fract(vec3((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y, (p3.y + p3.z) * p3.x));
}

// Raw simplex implementation by candycat
// Source: https://www.shadertoy.com/view/4sc3z2
float SimplexNoiseRaw(vec3 pos)
{
    const float K1 = 0.333333333;
    const float K2 = 0.166666667;
    
    vec3 i = floor(pos + (pos.x + pos.y + pos.z) * K1);
    vec3 d0 = pos - (i - (i.x + i.y + i.z) * K2);
    
    vec3 e = step(vec3(0.0), d0 - d0.yzx);
	vec3 i1 = e * (1.0 - e.zxy);
	vec3 i2 = 1.0 - e.zxy * (1.0 - e);
    
    vec3 d1 = d0 - (i1 - 1.0 * K2);
    vec3 d2 = d0 - (i2 - 2.0 * K2);
    vec3 d3 = d0 - (1.0 - 3.0 * K2);
    
    vec4 h = max(0.6 - vec4(dot(d0, d0), dot(d1, d1), dot(d2, d2), dot(d3, d3)), 0.0);
    vec4 n = h * h * h * h * vec4(dot(d0, hash33(i)), dot(d1, hash33(i + i1)), dot(d2, hash33(i + i2)), dot(d3, hash33(i + 1.0)));
    
    return dot(vec4(31.316), n);
}

float SimplexNoise(
    vec3  pos,
    int octaves,
    float scale,
    float persistence)
{
    float final        = 0.0;
    float amplitude    = 1.0;
    float maxAmplitude = 0.0;
    
    for(int i = 0; i < octaves; ++i)
    {
        final        += SimplexNoiseRaw(pos * scale) * amplitude;
        scale        *= 2.0;
        maxAmplitude += amplitude;
        amplitude    *= persistence;
    }
    
    return (final / maxAmplitude);
}


void main() {

    float scaledTime = time / 6;

    int currentNormalIndex = 0;
    vec2 fragPos2D = vec2(0);
    for (int i = 0; i < 3; ++i) {
        if (Normal[i] != 0) continue;
        fragPos2D[currentNormalIndex++] = FragPos[i];
    }

    // float noiseValue = SimplexNoise(vec3(fragPos2D*2, scaledTime), 3, 1.0, 0.5);
    // noiseValue = (noiseValue  + 1.0) * 0.5;
    // noiseValue *= 0.3;
    //noiseValue = 0;

    Material material = materials[int(round(Id))];
    // material.ambient += noiseValue;
    // material.diffuse += noiseValue;

    // ambient
    vec3 ambient = light.ambient * material.ambient;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    //vec3 norm = normalize(Normal + noiseValue);
    
    vec3 lightDir = normalize(light.position - FragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
    
    vec3 result = ambient + diffuse + specular;

    // Ambient occlusion
    //TODO: trouver une manière de lisser l'occlusion ambiente    
    result *= AO;

    outFragColor = vec4(result, 1.0);
    // if (AO == 0) outFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    // else if (AO <= 1) outFragColor = vec4(AO, 0.0, 0.0, 1.0);
    // else if (AO <= 2) outFragColor = vec4(0.0, AO/2.0, 0.0, 1.0);
    // else if (AO <= 3) outFragColor = vec4(0.0, 0.0, AO/3.0, 1.0);
    // outFragColor = vec4(AO/3.0, AO/3.0, AO/3.0, 1.0);
}
