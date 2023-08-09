#extension GL_GOOGLE_include_directive : enable

#include "octreeHelper.frag"

//TODO: Add uniform with world informations
layout (set = 0, binding = 1) uniform WorldOctreeInformations {
    ivec3 centerChunkPosition;
};

//Octree info (need uniforms...)
uint maxDepth = 5;
uint maxLen = 32;
float voxelSize = 0.5;

#define RAYTRACING_CHUNK_RANGE 1
#define RAYTRACING_CHUNK_PER_AXIS (RAYTRACING_CHUNK_RANGE * 2 + 1)
#define RAYTRACING_SVO_SIZE (RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS)

#define CENTER_OFFSET ivec3(RAYTRACING_CHUNK_RANGE, RAYTRACING_CHUNK_RANGE, RAYTRACING_CHUNK_RANGE)

layout(std430, binding = 2) readonly buffer SsboOctree {
    int octreeData[];
} ssboOctree[RAYTRACING_SVO_SIZE];



uint depthStep(inout uvec3 position, inout uint currentLen) {

    currentLen /= 2;    

    uvec3 mask = uvec3(greaterThanEqual(position, uvec3(currentLen)));
    position -= mask * currentLen;
    return uint(dot(mask, uvec3(1, 2, 4)));

}

struct getResult {
    int voxel;
    uint depth;
};

getResult octreeGet(int ssboIndex, uvec3 position) {
    
    uint currentIndex = 0;
    uint currentDepth = 0;
    uint currentLen = maxLen;

    while ( true ) {

        //This should never happen, the precedent test should always be true when we're at the last depth.
        //TODO: remove this security
        if (currentDepth == maxDepth) return getResult( -1, maxDepth );

        uint indexOffset = depthStep(position, currentLen);
        currentIndex += indexOffset;

        //indicate index of the next depth
        int currentValue = ssboOctree[ssboIndex].octreeData[currentIndex]; //TODO: trouver un moyen d'avoir plusieurs octreeData et choisir le bon en parametre de la fonction

        //If true, we found the value of the given position
        if (currentValue <= 0) return getResult( -currentValue, currentDepth );

        //If value not reached, we go to the next memory chunk
        currentIndex = currentValue;

        ++currentDepth;

    }

}

struct octreeRayCastResult {
    int returnCode;
    float traveledDistance;
    int voxel;
    vec3 normal;
};

octreeRayCastResult octreeCastRay(int ssboIndex, inout vec3 position, in vec3 direction, float maxDistance) {
    
    //Positive or negative direction
    vec3 directionSign = step(0.0, direction) * 2.0 - 1.0;

    //Index of the first voxel
    ivec3 voxelPosition = getIntPosition(position, directionSign);

    getResult voxelAndDepth = getResult(0, 0);
    float traveledDistance = 0.0f;

    shortestResult traveledAndIndex;

    voxelAndDepth = octreeGet(ssboIndex, voxelPosition);

    const float depthToLen_[5] = float[5](16, 8, 4, 2, 1);

    int counttempo = 0;
    while(voxelAndDepth.voxel == 0) {

        if (counttempo > 0) {

            if (voxelPosition.z == 0){
                    
                if (position.z < 0.001) {
                    return octreeRayCastResult(6, traveledDistance, 0, vec3(0, 0, 0));
                }
                else {
                    return octreeRayCastResult(5, traveledDistance, 0, vec3(0, 0, 0));
                }

            }
            else
                return octreeRayCastResult(5, traveledDistance, 0, vec3(0, 0, 0));
        }
        
        float leafSize = depthToLen_[voxelAndDepth.depth];

        //Required time to exit the full voxel along each axis.
        traveledAndIndex = getShortestTraveledDistanceAndIndex(position, direction, directionSign, leafSize);             
        
        //Compute position offset and add it to the current position to find the new current position
        vec3 positionOffset = direction * traveledAndIndex.traveledDistance;
        position += positionOffset;

        position = round(position * 1000.0) / 1000.0;

        //Add the traveled distance to the total traveled distance
        traveledDistance += traveledAndIndex.traveledDistance;

        //Compute the new voxel position
        voxelPosition = getIntPosition(position, directionSign);

        if (traveledDistance > maxDistance) return octreeRayCastResult(1, traveledDistance, 0, vec3(0, 0, 0));

        //Test if we went out of bound
        //Note: 2: x >= len, 3: x < 0, 4: y >= len, 5: y < 0, 6: z >= len, 7: z < 0
        int pos = -1;
        int totalPlus = 0;
        for (int i = 0; i < 3; ++i) {
            if (voxelPosition[i] >= int(maxLen)){
                pos = 2 + i*2;
                totalPlus++;
                //return octreeRayCastResult(2 + i*2, traveledDistance, 0, vec3(0, 0, 0));
            }
            if (voxelPosition[i] < 0) {
                totalPlus++;
                pos = 2 + i*2 + 1;
                //return octreeRayCastResult(2 + i*2 + 1, traveledDistance, 0, vec3(0, 0, 0));
            }
        }

        if (totalPlus > 0){
            return octreeRayCastResult(7, traveledDistance, 0, vec3(0, 0, 0));

            // if (counttempo == 0) {

            // }
            // else {

            // }

            if (totalPlus > 1) {
                return octreeRayCastResult(6, traveledDistance, 0, vec3(0, 0, 0));
            }
            else {
                return octreeRayCastResult(7, traveledDistance, 0, vec3(0, 0, 0));
            }

        }

        // if (voxelPosition.x >= int(maxLen)) {
        //     return octreeRayCastResult(2, traveledDistance, 0, vec3(0, 0, 0));
        // }
        // else if (voxelPosition.x < 0) {
        //     return octreeRayCastResult(3, traveledDistance, 0, vec3(0, 0, 0));
        // }


        voxelAndDepth = octreeGet(ssboIndex, voxelPosition);

        ++counttempo;
    }

    //We only arrive here if found voxel is true

    //Compute normal
    vec3 normal = vec3(0.0);
    normal[traveledAndIndex.index] = -directionSign[traveledAndIndex.index];

    return octreeRayCastResult(0, traveledDistance, voxelAndDepth.voxel, normal);

}

struct WorldRayCastResult {
    int voxel;
    vec3 hitPosition;
    vec3 normal;
    float dist;
};

//TODO: mettre les bon in
WorldRayCastResult worldCastRay(vec3 position, vec3 direction, float maxDistance) {
    
    position /= voxelSize;
    maxDistance /= voxelSize;

    // rayDirection += 0.0; //To prevent negative zero
    vec3 directionSign = step(0.0, direction) * 2.0 - 1.0;


    LocalPositionResult localPositionResult = getChunkLocalPosition(position, maxLen, 1.0);

    // localPositionResult.chunkPosition = ivec3(0, 0, 0);
    // localPositionResult.localPosition = vec3(0, 16, 16);

    //Store result between iterations
    octreeRayCastResult rayCastRes;
    rayCastRes.voxel = 7;

    //Loop here normaly...
    float totalTraveledDistance = 0.0f;

    //debug
    int maxIter = 10;
    int countIter = 0;

    while (totalTraveledDistance < maxDistance) {
        
        --maxIter;
        // if (maxIter <= 0) break;

        //TODO:
        ivec3 centeredChunkPosition = (localPositionResult.chunkPosition - centerChunkPosition) + CENTER_OFFSET;

        if (centeredChunkPosition.x < 0) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        if (centeredChunkPosition.y < 0) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        if (centeredChunkPosition.z < 0) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );

        if (centeredChunkPosition.x >= RAYTRACING_CHUNK_PER_AXIS) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        if (centeredChunkPosition.y >= RAYTRACING_CHUNK_PER_AXIS) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        if (centeredChunkPosition.z >= RAYTRACING_CHUNK_PER_AXIS) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );

        int ssboIndex = centeredChunkPosition.x * (RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS) + centeredChunkPosition.y * RAYTRACING_CHUNK_PER_AXIS + centeredChunkPosition.z;
        //Up to here

        // if (maxIter < 9) ssboIndex = 13;
        
        if (ssboOctree[ssboIndex].octreeData.length() == 0) {
            
            if (countIter > 0)
                return WorldRayCastResult( 4, vec3(0,0,0), vec3(0,0,0), 0);

            //TODO: changer le type pour mettre une majuscule
            shortestResult res = getShortestTraveledDistanceAndIndex(localPositionResult.localPosition, direction, directionSign, maxLen);

            vec3 positionOffset = direction * res.traveledDistance;
            localPositionResult.localPosition += positionOffset;

            if (totalTraveledDistance + res.traveledDistance > maxDistance) rayCastRes.returnCode = 1;
            else rayCastRes.returnCode = 2 + res.index * 2 + int(directionSign[res.index] < 0);

            //TODO: better ?
            rayCastRes.traveledDistance = res.traveledDistance;
            rayCastRes.voxel = 0;

            //TODO: copier la partie !processable de castSVORay dans World.cpp
            //break;
        }
        else {
            if (countIter > 0)
                return WorldRayCastResult( 5, vec3(0,0,0), vec3(0,0,0), 0);

            rayCastRes = octreeCastRay(ssboIndex, localPositionResult.localPosition, direction, maxDistance - totalTraveledDistance);
            // break;
        }

        // return WorldRayCastResult( 6, vec3(0,0,0), vec3(0,0,0), 0);

        totalTraveledDistance += rayCastRes.traveledDistance;

        if (rayCastRes.returnCode < 2) {
            return WorldRayCastResult( 7, vec3(0,0,0), vec3(0,0,0), 0);
            break;
            //TODO: peut être faire un comportement différent entre 0 et 1 ? (break dans 1 et return dans 2 pour ne pas calculer les positions par exemple ?)
        }
        else {
            
            if (rayCastRes.returnCode == 7) {
                return WorldRayCastResult( 5, vec3(0,0,0), vec3(0,0,0), 0);
            }
            else {
                
                if (rayCastRes.returnCode == 5) //TODO: voir pk
                    return WorldRayCastResult( 6, vec3(0,0,0), vec3(0,0,0), 0);

                return WorldRayCastResult( 4, vec3(0,0,0), vec3(0,0,0), 0);
            }

            //TODO: le mettre en global constante
            ivec2 codeToOffset[6] = ivec2[6]( ivec2(0, 1), ivec2(0, -1), ivec2(1, 1), ivec2(1, -1), ivec2(2, 1), ivec2(2, -1) );

            ivec2 newChunkOffset = codeToOffset[rayCastRes.returnCode - 2];
            localPositionResult.localPosition[newChunkOffset[0]] = ( (-newChunkOffset[1] + 1) / 2 ) * maxLen; //This calcul will transform 1 to 0 and -1 to 1
            localPositionResult.chunkPosition[newChunkOffset[0]] += newChunkOffset[1];

        }
        
        ++countIter;
    }


    return WorldRayCastResult( rayCastRes.voxel, (localPositionResult.localPosition - localPositionResult.worldPositionOffset) * voxelSize, rayCastRes.normal, totalTraveledDistance * voxelSize );

}
