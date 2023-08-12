

vec3 computeAmbient(in Material material) {
    return light.ambient * material.ambient;
}

vec3 computeDiffuse(in Material material, in vec3 normal, in vec3 lightDir) {
    float diff = max(dot(normal, lightDir), 0.0);
    return light.diffuse * (diff * material.diffuse);
}

vec3 computeSpecular(in Material material, in vec3 viewDir, in vec3 normal, in vec3 lightDir) {
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    return light.specular * (spec * material.specular);  
}
