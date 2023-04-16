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

    Material material = materials[int(round(Id))];

    // ambient
    vec3 ambient = light.ambient * material.ambient;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    
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
