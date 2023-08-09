#extension GL_GOOGLE_include_directive : require

#include "octree.frag"
#include "lightHelper.frag"

#define PI 3.1415926538

//DEBUG TO REMOVE

struct RayAABBResult {
    bool intersect;
    float dist;
};

RayAABBResult box(vec3 ray_origin, vec3 ray_dir, vec3 minpos, vec3 maxpos) {
  vec3 inverse_dir = 1.0 / ray_dir;
  vec3 tbot = inverse_dir * (minpos - ray_origin);
  vec3 ttop = inverse_dir * (maxpos - ray_origin);
  vec3 tmin = min(ttop, tbot);
  vec3 tmax = max(ttop, tbot);
  vec2 traverse = max(tmin.xx, tmin.yz);
  float traverselow = max(traverse.x, traverse.y);
  traverse = min(tmax.xx, tmax.yz);
  float traversehi = min(traverse.x, traverse.y);
  return RayAABBResult(traversehi > max(traverselow, 0.0), traverselow);
}

//DEBUG END


//TODO: Smooth shadow
bool inShadow (vec3 startPosition, vec3 offset, vec3 lightDir, float lightDist) {
    //Shadow calculation
    WorldRayCastResult shadowRayCastResult = worldCastRay( startPosition + offset, lightDir, lightDist );
    return shadowRayCastResult.voxel != 0;
}

//TODO: rassembler tout les trucs en rapport avec l'AO dans un même frag file
ivec4 edges( in vec3 vos, in vec3 nor, in vec3 dir ) {
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

ivec4 corners( in vec3 vos, in vec3 nor, in vec3 dir ) {
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

//TODO: verify impact on performance of AO
float computeAO(vec3 norm, vec3 hitDir, vec3 hitVoxelChunkPos, vec2 uv) {

    //TODO: voir si utile
    roundPosition(hitVoxelChunkPos);

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

vec3 applyFog(vec3 color, vec3 fogColor, float colorDistance, float maxDistance) {

    //Select the percentage of maximum distance at which the interpolation start. A value >= to 1.0 mean brutal fog
    const float fogPercentStart = 0.7;
    float fogStartDistance = fogPercentStart * maxDistance;

    //Calculate fogFactor
    float fogFactor = (colorDistance - fogStartDistance) / (maxDistance - fogStartDistance);
    fogFactor = clamp(fogFactor, 0, 1);
    
    /// Select the good easing function
    // Cosine interpolation
    float alpha = (1.0 - cos(fogFactor * PI)) / 2.0;

    return mix(color, fogColor, alpha);

}

//Note: UV are like the texture position of the hitted voxel face
vec2 computeHitUV(vec3 hitPos, vec3 hitDir, vec3 hitVoxelChunkPos) {
    vec3 uvw = (hitPos - (hitVoxelChunkPos + voxelSize * 0.5)) / voxelSize;
    return vec2( dot(hitDir.yzx, uvw), dot(hitDir.zxy, uvw) ) + 0.5;
}

struct RayCastingResult {
    vec3 color;
    vec3 lightColor;
};

RayCastingResult castRay(vec3 position, vec3 direction, float maxDistance) {
    
    //Parameter background color
    const vec3 skyColor = vec3(0.5294, 0.8078, 0.9216); //Sky
    

    RayCastingResult result = RayCastingResult(skyColor, skyColor);

    //TODO: add distance as parameter of the Shader
    WorldRayCastResult rayCastResult = worldCastRay( position, direction, maxDistance );

    //TODO: Enlever ça mais le garder quelque part, ça fais un mirroir
    // if (rayCastResult.voxel == 4) {

    //     float dotted = dot(direction, rayCastResult.normal);
    //     vec3 newDir = direction + 2.0 * (rayCastResult.normal * dotted);

    //     newDir = -2 * dotted * rayCastResult.normal + direction;

    //     rayCastResult = worldCastRay( rayCastResult.hitPosition, newDir, maxDistance );
    // }

    //TODO: move this to a loop of sun, and select the goo ray casting algorithm
    // Ray rayTest = Ray(position, direction);
    // Box testBox = Box(sun.position, vec3(0.5), -vec3(0.5), mat3(1.0));

    //TODO: have a global ray casting function that compare everything
    float distRez = 0;
    vec3 sunNormal = vec3(0.0);
    vec3 voxelRayColor = vec3(1.0);
    // bool toucheeed = ourIntersectBoxCommon(testBox, rayTest, distRez, norr, false, false, 1.0/direction);
    RayAABBResult sunRayResult =  box(position, direction, sun.position - 5, sun.position + 5);


    for (int i = 0; i < simpleVoxels.length(); ++i) {
        //return result;
        vec3 voxelPos = simpleVoxels[i].position;
        vec3 voxelExtents = simpleVoxels[i].extents;
        RayAABBResult voxelRayResult = box(position, direction, voxelPos - voxelExtents, voxelPos + voxelExtents);
        if ( voxelRayResult.intersect && (!sunRayResult.intersect || voxelRayResult.dist < sunRayResult.dist) ) {
            sunRayResult = voxelRayResult;
            voxelRayColor = simpleVoxels[i].material.ambient;
        }
    }

    if (sunRayResult.intersect && (rayCastResult.voxel == 0 || sunRayResult.dist < rayCastResult.dist)) {
        result.color = voxelRayColor;
        result.lightColor = vec3(1.0);
    }
    else if (rayCastResult.voxel != 0) {
        
        //TODO: Pour le moment un voxel = pas de light color, peut être modifier ce comportement
        result.lightColor = vec3(0.0);
        
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
        bool shadow = inShadow( rayCastResult.hitPosition /* + rayCastResult.normal * 0.001 */, vec3(0), lightDir, min(lightDist, maxDistance) );
        if (shadow) illuminated = 0.0;
        else illuminated = 1.0;// - (lightDist / maxDistance);

        //Tempo without shadow...
        // float illuminated = 1.0;

        //Fun
        float distFromEdge = max(abs(uv.x - 0.5), abs(uv.y - 0.5)) * 2;
        float scaledTime = time * 3;

        Material material = materials[rayCastResult.voxel];

        // Classic Phong Lighting
        vec3 ambient = computeAmbient(material);
        vec3 diffuse = computeDiffuse(material, rayCastResult.normal, lightDir);
        vec3 specular = computeSpecular(material, normalize(position - rayCastResult.hitPosition), rayCastResult.normal, lightDir);
        
        //With shadow
        vec3 constantColor = ambient;
        vec3 illuminatedColor = (diffuse + specular);

        //TODO: Find the perfect color picking
        result.color = (constantColor + illuminatedColor * illuminated) * AO;
        // color = (ambient + light.diffuse * illuminated * material.diffuse + specular) * AO;
        // color = material.ambient * AO;

        // Could be a cool idea
        // result.lightColor = illuminatedColor * illuminated * AO;

        // if (illuminated == 0.0) {
        //     color = vec3(0.0);
        // }

        if (rayCastResult.voxelWorldPosition == voxelCursorPosition && rayCastResult.normal == faceCursorNormal) {
            if ( distFromEdge >= 0.925 + cos(scaledTime)*0.035 ) {
                result.color = vec3(1.0);
            }
        }

        //We apply fog on the found color
        result.color = applyFog(result.color, skyColor, rayCastResult.dist, maxDistance);

    }
    
    return result;
}
