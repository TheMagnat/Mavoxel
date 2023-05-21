#version 450 core

//PRAMS
#define RAY_DISTANCE 128



//Structures definition
struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float shininess;
};

struct Sun {
    vec3 position;
};

Material light = Material(
    vec3(0.1f, 0.1f, 0.1f),
    vec3(0.5f, 0.5f, 0.5f),
    vec3(1.0f, 1.0f, 1.0f),
    32.0
);

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


//Inpute / Outputs
layout (location = 0) out vec4 outFragColor;
layout (location = 0) in vec2 TexPos;

//Uniforms
layout (set = 0, binding = 0) uniform RayCastInformations {
    float xRatio;
    Camera camera;
    Sun sun;
};

#define TEXTURES_LEN 5
#define TEXTURES_TOTAL_LEN (TEXTURES_LEN * TEXTURES_LEN * TEXTURES_LEN)
#define CENTER_OFFSET ivec3(2, 2, 2)

layout(binding = 1) uniform usampler3D chunkTexture_one;

// uniform ivec3 centerChunkPosition;
// uniform usampler3D chunkTextures[TEXTURES_TOTAL_LEN];

ivec3 centerChunkPosition = ivec3(0, 0, 0);
float voxelSize = 0.5;
int chunSize = 32;

float trueChunkSize = chunSize * voxelSize;
float halfChunkSize = trueChunkSize / 2.0;

//Logic
uint findVoxel(vec3 voxelPosition) {

    vec3 positionSign = sign(voxelPosition);
    
    ivec3 chunkPosition = ivec3( (voxelPosition + halfChunkSize * positionSign) / trueChunkSize );

    ivec3 centeredChunkPosition = (chunkPosition - centerChunkPosition) + CENTER_OFFSET;

    if (centeredChunkPosition.x < 0) return 0;
    if (centeredChunkPosition.y < 0) return 0;
    if (centeredChunkPosition.z < 0) return 0;

    if (centeredChunkPosition.x >= TEXTURES_LEN) return 0;
    if (centeredChunkPosition.y >= TEXTURES_LEN) return 0;
    if (centeredChunkPosition.z >= TEXTURES_LEN) return 0;

    int indexInTextureArray = centeredChunkPosition.x * TEXTURES_LEN * TEXTURES_LEN + centeredChunkPosition.y * TEXTURES_LEN + centeredChunkPosition.z;
    
    // if ( chunkPosition != ivec3(0, 0, 0) ) return 0;
    //Verifier que la chunk position existe, et trouver la texture associé

    ivec3 voxelChunkPosition = ivec3( ( (voxelPosition + halfChunkSize) - chunkPosition * trueChunkSize ) / voxelSize );

    //return texelFetch(chunkTextures[indexInTextureArray], voxelChunkPosition, 0).r;
    return texelFetch(chunkTexture_one, voxelChunkPosition, 0).r;

}

//Note: ray must be normalized
uint castWorldRay(vec3 eye, vec3 ray, out float dist, out vec3 norm, out vec3 hitVoxelChunkPos) {
    
    //TODO: move
    float maxDistance = 100.0;

    if (ray.x == 0.0) ray.x = 0.0;
    if (ray.y == 0.0) ray.y = 0.0;
    if (ray.z == 0.0) ray.z = 0.0;
    
    vec3 pos = vec3(floor(eye / voxelSize) * voxelSize);
    vec3 ri = 1.0 / ray; //Inversed ray
    vec3 rs = sign(ray); //Sign of ray

    vec3 ris = ri * rs; 
    vec3 dis = (pos - eye + (voxelSize * 0.5) + rs * (voxelSize * 0.5)) * ri;
    
    vec3 dim = vec3(0.0);
    for (int i = 0; i < RAY_DISTANCE; ++i) {

        //TODO: regarder si dot(dis - (ris * voxelSize), dim) dépasse distance max
        dist = dot(dis - (ris * voxelSize), dim);
        // if (dist > maxDistance) return 0u;

        hitVoxelChunkPos = pos + (voxelSize * 0.5);
        uint foundVoxel = findVoxel( hitVoxelChunkPos );
        if (foundVoxel != 0u) {
            norm = -dim * rs;
            return foundVoxel;
        }

        dim = step(dis, dis.yzx);
		dim *= (1.0 - dim.zxy);

        dis += dim * ris * voxelSize;
        pos += dim * rs * voxelSize;
    }

	return 0u;
}

ivec4 edges( in vec3 vos, in vec3 nor, in vec3 dir )
{
	vec3 v1 = vos + (nor * voxelSize) + (dir.yzx * voxelSize);
	vec3 v2 = vos + (nor * voxelSize) - (dir.yzx * voxelSize);
	vec3 v3 = vos + (nor * voxelSize) + (dir.zxy * voxelSize);
	vec3 v4 = vos + (nor * voxelSize) - (dir.zxy * voxelSize);

	ivec4 res = ivec4(0);
	if( findVoxel(v1) != 0u ) res.x = 1;
	if( findVoxel(v2) != 0u ) res.y = 1;
	if( findVoxel(v3) != 0u ) res.z = 1;
	if( findVoxel(v4) != 0u ) res.w = 1;

	return res;
}

ivec4 corners( in vec3 vos, in vec3 nor, in vec3 dir )
{
	vec3 v1 = vos + (nor * voxelSize) + (dir.yzx * voxelSize) + (dir.zxy * voxelSize);
	vec3 v2 = vos + (nor * voxelSize) - (dir.yzx * voxelSize) + (dir.zxy * voxelSize);
	vec3 v3 = vos + (nor * voxelSize) - (dir.yzx * voxelSize) - (dir.zxy * voxelSize);
	vec3 v4 = vos + (nor * voxelSize) + (dir.yzx * voxelSize) - (dir.zxy * voxelSize);

	ivec4 res = ivec4(0);
	if( findVoxel(v1) != 0u ) res.x = 1;
	if( findVoxel(v2) != 0u ) res.y = 1;
	if( findVoxel(v3) != 0u ) res.z = 1;
	if( findVoxel(v4) != 0u ) res.w = 1;

	return res;
}

float computeAO (vec3 hitPos, vec3 norm, vec3 hitDir, vec3 hitVoxelChunkPos) {
    
    vec3 hitDirSized = hitDir * voxelSize;
    vec3 normSized = norm * voxelSize;

    ivec4 ed = edges( hitVoxelChunkPos, norm, hitDir );
    ivec4 co = corners( hitVoxelChunkPos, norm, hitDir );

    vec3 uvw = (hitPos - hitVoxelChunkPos) / voxelSize;
    vec2 uv = vec2( dot(hitDir.yzx, uvw), dot(hitDir.zxy, uvw) ) + 0.5;

    float occ = 0.0; 
    vec4 allCorners = vec4(0.0);

    //TODO: si les 2 edge sont true, ça doit valoir 3
    allCorners.x = ed.x + ed.z; //Haut droite
    allCorners.y = ed.z + ed.y; //Bas droite
    allCorners.z = ed.y + ed.w; //Bas gauche
    allCorners.w = ed.w + ed.x; //Haut gauche

    allCorners.x += allCorners.x == 2 ? 1 : co.x; //Haut droite
    allCorners.y += allCorners.y == 2 ? 1 : co.y; //Bas droite
    allCorners.z += allCorners.z == 2 ? 1 : co.z; //Bas gauche
    allCorners.w += allCorners.w == 2 ? 1 : co.w; //Haut gauche


    // vec4 curve = vec4(0.0, 0.6, 0.8, 1.0);
    // allCorners.x = curve[int(allCorners.x)];
    // allCorners.y = curve[int(allCorners.y)];
    // allCorners.z = curve[int(allCorners.z)];
    // allCorners.w = curve[int(allCorners.w)];

    float middleDown = mix(allCorners.z, allCorners.y, uv.y);
    float middleUp = mix(allCorners.w, allCorners.x, uv.y);
    float vertical = mix(middleDown, middleUp, uv.x);

    float middleLeft = mix(allCorners.z, allCorners.w, uv.x);
    float middleRight = mix(allCorners.y, allCorners.x, uv.x);
    float lateral = mix(middleLeft, middleRight, uv.y);

    return 1.0 - (vertical/3.0 * lateral/3.0) * 0.8; //TODO: rendre cette valeur parametrable
}

float computeShadow (vec3 startPosition, vec3 offset, vec3 lightDir) {
    //Shadow calculation
    float shadowHitDist;
    vec3 shadowHitNorm;
    vec3 shadowHitVoxelPosition;
    uint hitToSunId = castWorldRay( startPosition + offset, lightDir, shadowHitDist, shadowHitNorm, shadowHitVoxelPosition );
    return hitToSunId != 0u ? 1.0 : 0.0;
}

vec3 computeAmbient(Material material) {
    return light.ambient * material.ambient;
}

vec3 computeDiffuse(Material material, vec3 normal, vec3 lightDir) {
    float diff = max(dot(normal, lightDir), 0.0);
    return light.diffuse * (diff * material.diffuse);
}

vec3 computeSpecular(Material material, vec3 viewDir, vec3 normal, vec3 lightDir) {
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    return light.specular * (spec * material.specular);  
}

vec3 castRay(vec3 position, vec3 direction) {
    
    vec3 color = vec3(0.5294, 0.8078, 0.9216);
    //color = direction;
    vec3 hitPosition;

    float dist;
    vec3 norm;
    vec3 hitVoxelChunkPos; //Hit voxel position
    uint hitId = castWorldRay( position, direction, dist, norm, hitVoxelChunkPos );
    if (hitId != 0u) {
        
        //Hit position
        vec3 hitPos = position + dist * direction;
        vec3 hitDir = -norm*sign(direction);
        vec3 lightDir = normalize(sun.position - hitPos);
                
        // Ambient occlusion
        float AO = computeAO(hitPos, norm, hitDir, hitVoxelChunkPos);


        //Calculate shadow ray
        float shadow = computeShadow(hitPos, (0.000001 * dist) * norm, lightDir);
        float illuminated = 1.0 - shadow;

        Material material = materials[hitId];

        // ambient
        vec3 ambient = computeAmbient(material);
        
        // diffuse                 
        vec3 diffuse = computeDiffuse(material, norm, lightDir);
        
        // specular
        vec3 specular = computeSpecular(material, normalize(position - hitPos), norm, lightDir);
        

        float shade = dot(norm, lightDir);
        
        // float shadow = 0.7;
        // float lightValue = (1.0 + shadow * (illuminated * max(shade, 0.0) - 1.0)) * (1.0 - max(-shade, 0.0));


        //float testt = distance(uv, hitVoxelChunkPos);
        //float testt = length(hitPos);
        


        color = (ambient + light.diffuse * (illuminated * material.diffuse) + specular) * AO;
    }
    
    return color;
}

vec3 getRayTarget() {
    vec2 uv = TexPos * 2 - 1.0;
    uv.x *= xRatio;
    //vec2 ndc = (2.0 * TexPos - screenSize) / screenSize;
    //vec2 ndc = (2.0 * TexPos - screenSize) / screenSize;
    vec3 near = camera.position + camera.front * 2.5;
    vec3 right = camera.right * uv.x;
    vec3 up = -camera.up * uv.y;
    return near + right + up;
}

vec3 getRayDir(vec3 rayTarget) {
    return normalize(rayTarget - camera.position);
}

//Main function
void main() {

    vec3 rayTarget = getRayTarget();
    vec3 rayDirection = getRayDir(rayTarget);

    

    //outFragColor = vec4(TexPos.x, TexPos.y, 0.0, 1.0);
    // vec2 uv = vec2(TexPos.x * 2, TexPos.y * 2) - 1.0;
    // uv.x *= xRatio;

    // vec3 rayOrigin = camera.position;
    // vec3 rayDirection = normalize(vec3(uv.x, uv.y,1));

    vec3 castRayOutput = castRay(camera.position, rayDirection);

    // float distFromCenter = length(uv);
    // outFragColor = vec4(vec3(distFromCenter), 1.0);

    outFragColor = vec4(castRayOutput, 1.0);
    //outFragColor = vec4(vec3()
}
