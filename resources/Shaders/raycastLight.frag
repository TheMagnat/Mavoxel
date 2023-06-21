#extension GL_GOOGLE_include_directive : require
#include "octree.frag"
#include "materials.frag"
#include "lightHelper.frag"

//Structures definition
struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
};

//TODO: remove sun, create light with it's attributes
struct Sun {
    vec3 position;
};

//Uniforms
layout (set = 0, binding = 0) uniform RayCastInformations {
    float xRatio;
    Camera camera;
    Sun sun;
    float time;
    vec3 voxelCursorPosition;
    vec3 faceCursorNormal;
};

//TODO: Smooth shadow
bool inShadow (vec3 startPosition, vec3 offset, vec3 lightDir, float lightDist) {
    //Shadow calculation
    WorldRayCastResult shadowRayCastResult = worldCastRay( startPosition + offset, lightDir, lightDist );
    return shadowRayCastResult.voxel != 0;
}

ivec4 edges( in vec3 vos, in vec3 nor, in vec3 dir )
{
	vec3 v1 = vos + nor + dir.yzx;
	vec3 v2 = vos + nor - dir.yzx;
	vec3 v3 = vos + nor + dir.zxy;
	vec3 v4 = vos + nor - dir.zxy;

	ivec4 res = ivec4(0);
	if( octreeGetWorld(v1).voxel != 0 ) res.x = 1;
	if( octreeGetWorld(v2).voxel != 0 ) res.y = 1;
	if( octreeGetWorld(v3).voxel != 0 ) res.z = 1;
	if( octreeGetWorld(v4).voxel != 0 ) res.w = 1;

	return res;
}

ivec4 corners( in vec3 vos, in vec3 nor, in vec3 dir )
{
	vec3 v1 = vos + nor + dir.yzx + dir.zxy;
	vec3 v2 = vos + nor - dir.yzx + dir.zxy;
	vec3 v3 = vos + nor - dir.yzx - dir.zxy;
	vec3 v4 = vos + nor + dir.yzx - dir.zxy;

	ivec4 res = ivec4(0);
	if( octreeGetWorld(v1).voxel != 0 ) res.x = 1;
	if( octreeGetWorld(v2).voxel != 0 ) res.y = 1;
	if( octreeGetWorld(v3).voxel != 0 ) res.z = 1;
	if( octreeGetWorld(v4).voxel != 0 ) res.w = 1;

	return res;
}

//Note: UV are like the texture position of the hitted voxel face
vec2 computeHitUV(vec3 hitPos, vec3 hitDir, vec3 hitVoxelChunkPos) {
    vec3 uvw = (hitPos - (hitVoxelChunkPos + voxelSize * 0.5)) / voxelSize;
    return vec2( dot(hitDir.yzx, uvw), dot(hitDir.zxy, uvw) ) + 0.5;
}

//TODO: verify impact on performance of AO
float computeAO(vec3 norm, vec3 hitDir, vec3 hitVoxelChunkPos, vec2 uv) {

    ivec4 ed = edges( hitVoxelChunkPos, norm, hitDir );
    ivec4 co = corners( hitVoxelChunkPos, norm, hitDir );

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

vec3 castRay(vec3 position, vec3 direction, float maxDistance) {
    
    //Parameter background color
    vec3 color = vec3(0.5294, 0.8078, 0.9216);

    //TODO: add distance as parameter of the Shader
    WorldRayCastResult rayCastResult = worldCastRay( position, direction, maxDistance );

    //TODO: Enlever ça mais le garder quelque part, ça fais un mirroir
    // if (rayCastResult.voxel == 4) {

    //     float dotted = dot(direction, rayCastResult.normal);
    //     vec3 newDir = direction + 2.0 * (rayCastResult.normal * dotted);

    //     newDir = -2 * dotted * rayCastResult.normal + direction;

    //     rayCastResult = worldCastRay( rayCastResult.hitPosition, newDir, maxDistance );
    // }

    if (rayCastResult.voxel != 0) {

        
        //Hit position informations
        //TODO: better hitdir ?
        vec3 hitDir = -rayCastResult.normal * sign(direction);
        vec3 lightDir = normalize(sun.position - rayCastResult.hitPosition);
        float lightDist = length(sun.position - rayCastResult.hitPosition);
        vec2 uv = computeHitUV (rayCastResult.hitPosition, hitDir, rayCastResult.voxelWorldPosition);
                
        // Ambient occlusion
        float AO = computeAO(rayCastResult.normal, hitDir, rayCastResult.voxelWorldPosition / voxelSize, uv);


        //Calculate shadow ray
        //Note: dist is traveled distance
        float illuminated;
        bool shadow = inShadow( rayCastResult.hitPosition, vec3(0), lightDir, min(lightDist, maxDistance) );
        if (shadow) illuminated = 0.0;
        else illuminated = 1.0;// - (lightDist / maxDistance);

        //Tempo without shadow...
        // float illuminated = 1.0;

        //Fun
        float distFromEdge = max(abs(uv.x - 0.5), abs(uv.y - 0.5)) * 2;
        float scaledTime = time * 3;

        Material material = materials[rayCastResult.voxel];

        //Put selection under shadow
        // if (rayCastResult.voxelWorldPosition == voxelCursorPosition && rayCastResult.normal == faceCursorNormal) {
        //     if ( distFromEdge >= 0.925 + cos(scaledTime)*0.035 ) {
        //         material.ambient = vec3(0.1);
        //         material.diffuse = vec3(1.0);
        //         material.specular = vec3(1.0);
        //         material.shininess = 32.0;

        //         light.ambient = vec3(0.1);
        //         light.diffuse = vec3(1.0);
        //         light.specular = vec3(1.0);
        //         light.shininess = 32.0;
        //     }
        // }

        // Classic Phong Lighting
        vec3 ambient = computeAmbient(material);
        vec3 diffuse = computeDiffuse(material, rayCastResult.normal, lightDir);
        vec3 specular = computeSpecular(material, normalize(position - rayCastResult.hitPosition), rayCastResult.normal, lightDir);
        

        float shade = dot(rayCastResult.normal, lightDir);
        
        // float shadow = 0.7;
        // float lightValue = (1.0 + shadow * (illuminated * max(shade, 0.0) - 1.0)) * (1.0 - max(-shade, 0.0));


        //float testt = distance(uv, hitVoxelChunkPos);
        //float testt = length(rayCastResult.hitPosition);
        

        //TODO: add back AO
        // color = (ambient + diffuse + specular);// * AO;

        
        //With shadow
        vec3 constantColor = ambient;
        vec3 illuminatedColor = (diffuse + specular);

        //TODO: Find the perfect color picking
        color = (constantColor + illuminatedColor * illuminated) * AO;
        // color = (ambient + light.diffuse * illuminated * material.diffuse + specular) * AO;
        // color = material.ambient * AO;


        if (rayCastResult.voxelWorldPosition == voxelCursorPosition && rayCastResult.normal == faceCursorNormal) {
            if ( distFromEdge >= 0.925 + cos(scaledTime)*0.035 ) {
                color = vec3(1.0);
            }
        }
    }
    
    return color;
}
