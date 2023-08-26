#extension GL_GOOGLE_include_directive : require

#include "octree.frag"
#include "enhancingEffects.frag"
#include "lightHelper.frag"
#include "collisions.frag"


//DEBUG TO REMOVE
//DEBUG END


/**
 * hitType - Give informations about the type of collision encountered :
 *      - 0 : Nothing
 *      - 1 : World collision
 *      - 2 : Independent voxel collision
 *      - 3 : light voxel collision
 **/
struct RayCastResult {
    int hitType;
    vec3 hitPosition;
    vec3 normal;
    float dist;
    Material material;

    //For world only
    vec3 voxelWorldPosition;
};

RayCastResult castRay(vec3 position, vec3 direction, float maxDistance, bool isShadowRay) {

    RayCastResult result;

    result.hitType = 0;
    result.dist = maxDistance;

    WorldRayCastResult rayCastResult = worldCastRay( position, direction, maxDistance );
    if (rayCastResult.voxel != 0) {
        result.hitType = 1;
        result.hitPosition = rayCastResult.hitPosition;
        result.normal = rayCastResult.normal;
        result.dist = rayCastResult.dist;
        result.material = materials[rayCastResult.voxel];

        result.voxelWorldPosition = rayCastResult.voxelWorldPosition;
    }
    
    //Check all simple voxels...
    for (int i = 0; i < simpleVoxels.length(); ++i) {
        RayAABBResult voxelRayResult = box(position, direction, simpleVoxels[i].position - simpleVoxels[i].extents, simpleVoxels[i].position + simpleVoxels[i].extents);
        if ( voxelRayResult.intersect && (voxelRayResult.dist < result.dist) ) {
            result.hitType = 2;
            result.hitPosition = voxelRayResult.hitPosition;
            result.normal = voxelRayResult.normal;
            result.dist = voxelRayResult.dist;
            result.material = simpleVoxels[i].material;
        }
    }

    //Check all lights only if not a shadow ray...
    if (!isShadowRay) {
        RayAABBResult sunRayResult =  box(position, direction, sun.position - 5, sun.position + 5);
        if ( sunRayResult.intersect && (sunRayResult.dist < result.dist) ) {
            result.hitType = 3;
            result.hitPosition = sunRayResult.hitPosition;
            result.normal = sunRayResult.normal;
            result.dist = sunRayResult.dist;
            result.material = light;
        }
    }

    return result;
}

//TODO: Smooth shadow
bool inShadow (vec3 startPosition, vec3 lightDir, float lightDist) {
    //Shadow calculation
    RayCastResult shadowRayCastResult = castRay(startPosition, lightDir, lightDist, true);
    return shadowRayCastResult.hitType != 0;
}

struct RayTracingResult {
    vec3 color;
    vec3 lightColor;
    vec4 position;
};

RayTracingResult applyRayTracing(vec3 position, vec3 direction, float maxDistance) {
    
    //Parameter background color
    const vec3 skyColor = vec3(0.5294, 0.8078, 0.9216); //Sky
    
    RayTracingResult result = RayTracingResult(skyColor, skyColor, vec4(0.0));

    //TODO: add distance as parameter of the Shader
    RayCastResult rayCastResult = castRay(position, direction, maxDistance, false);


    //TODO: Enlever ça mais le garder quelque part, ça fais un mirroir
    // if (rayCastResult.voxel == 4) {

    //     float dotted = dot(direction, rayCastResult.normal);
    //     vec3 newDir = direction + 2.0 * (rayCastResult.normal * dotted);

    //     newDir = -2 * dotted * rayCastResult.normal + direction;

    //     rayCastResult = worldCastRay( rayCastResult.hitPosition, newDir, maxDistance );
    // }

    
    if (rayCastResult.hitType == 3) {
        result.color = rayCastResult.material.specular;
        result.lightColor = rayCastResult.material.specular;
        result.position = projection * view * vec4(rayCastResult.hitPosition, 1);
    }
    else if (rayCastResult.hitType == 1 || rayCastResult.hitType == 2) {
        
        //TODO: Pour le moment un voxel = pas de light color, peut être modifier ce comportement
        result.lightColor = vec3(0.0);
        result.position = projection * view * vec4(rayCastResult.hitPosition, 1);
        
        //Hit position informations
        //TODO: better hitdir ?
        vec3 hitDir = -rayCastResult.normal * sign(direction);
        vec3 lightDir = normalize(sun.position - rayCastResult.hitPosition);
        float lightDist = length(sun.position - rayCastResult.hitPosition);
        vec2 uv = computeHitUV (rayCastResult.hitPosition, hitDir, rayCastResult.voxelWorldPosition);
                
        // Ambient occlusion
        float AO = rayCastResult.hitType == 1 ? computeAO(rayCastResult.normal, hitDir, rayCastResult.voxelWorldPosition / voxelSize, uv) : 1.0;


        //Calculate shadow ray
        //Note: dist is traveled distance
        float illuminated;

        //There is some setup that show noise without the added bias...
        bool shadow = inShadow( rayCastResult.hitPosition /* + rayCastResult.normal * 0.000001 */, lightDir, min(lightDist, maxDistance) );
        if (shadow) illuminated = 0.0;
        else illuminated = 1.0;// - (lightDist / maxDistance);

        //Tempo without shadow...
        // float illuminated = 1.0;

        //Fun

        // Classic Phong Lighting
        vec3 ambient = computeAmbient(rayCastResult.material);
        vec3 diffuse = computeDiffuse(rayCastResult.material, rayCastResult.normal, lightDir);
        vec3 specular = computeSpecular(rayCastResult.material, normalize(position - rayCastResult.hitPosition), rayCastResult.normal, lightDir);
        
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

        // Selection effect (only in world)
        if (rayCastResult.hitType == 1) {
            float distFromEdge = max(abs(uv.x - 0.5), abs(uv.y - 0.5)) * 2;
            float scaledTime = time * 3;
            if (rayCastResult.voxelWorldPosition == voxelCursorPosition && rayCastResult.normal == faceCursorNormal) {
                if ( distFromEdge >= 0.925 + cos(scaledTime)*0.035 ) {
                    result.color = vec3(1.0);
                }
            }
        }

        //We apply fog on the found color
        result.color = applyFog(result.color, skyColor, rayCastResult.dist, maxDistance);

    }
    
    return result;
}
