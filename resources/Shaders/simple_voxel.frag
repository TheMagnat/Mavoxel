#version 450 core

layout(location = 0) out vec4 outFragColor;

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

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexPos;
layout(location = 3) in float AO;
layout(location = 4) in float Id;

layout(set = 0, binding = 1) uniform viewInformation {
    vec3 viewPos;
};
//uniform Material material;

layout(set = 0, binding = 2) uniform lightInformation {
    Light light;
};

// uniform float time;

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


void main() {

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
    // lightDir = vec3(0.0, 1.0, 0.0);

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

    //outFragColor = vec4(Normal, 1.0);
}
