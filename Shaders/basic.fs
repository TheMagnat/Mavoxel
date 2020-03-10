#version 330 core


struct Material {
    //Couleur des reflet sur chaque type de reflets
    //vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float shininess;
};
uniform Material material;

struct Light {
    //Position de la lumiere
    vec3 position;
    vec3 direction;
    
    //Intensité de la lumière sur chaque type de reflet
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    //Attenuation
    float constant;
    float linear;
    float quadratic;
};
uniform Light light;

uniform vec3 viewPos;

uniform float water;

//Venant du vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 texPos;

in float visibility;


//Couleur de sortie du pixel
out vec4 FragColor;


void main(){

    vec3 color;
    float d2;
    
    if(water == 1.0f){

        float delta = (FragPos.y+5.0f)/10.0f;
        d2 = 0;

        color = normalize(vec3(99, 203, 255));
    }
    else{

        float delta = (FragPos.y+15.0f)/30.0f;

        vec3 bas = normalize(vec3(139, 255, 82));
        vec3 haut = normalize(vec3(255, 151, 71));

        d2 = (1 - cos(delta * 3.141592))/2;

        color = bas * (1 - d2) + haut * d2;

    }


    


    //vec3 color = normalize(vec3(delta*0.7, 1 - delta*0.5, 0));

    vec3 colorAmbient = color;
    vec3 colorDiffuse = color*0.4;
    vec3 colorSpecular = vec3(((1 - d2)*0.3));
    
    // ambient
    vec3 ambient = light.ambient * colorAmbient;
    // diffuse
    
    //Ici c'est la normal normalement..
    vec3 norm = normalize(Normal);
    
    //vec3 lightDir = normalize(-light.direction);
    vec3 lightDir = normalize(light.position - FragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * colorDiffuse;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * colorSpecular;

    
    vec3 result = ambient + diffuse + specular;
    
    FragColor = vec4(result, 1.0);
    
    //FragColor = vec4(norm, 1.0);

    //FragColor = mix(vec4(0.52f, 0.83f, 1.0f, 1.0f), FragColor, visibility);
    
    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    
}


