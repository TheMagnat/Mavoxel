#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require

//Global information used by Octree algorithm and Octree helper algorithms
const ivec2 codeToOffset[6] = ivec2[6]( ivec2(0, 1), ivec2(0, -1), ivec2(1, 1), ivec2(1, -1), ivec2(2, 1), ivec2(2, -1) );

layout (set = 0, binding = 1) uniform WorldOctreeInformations {
    ivec3 centerChunkPosition;
    int maxDepth;
    int maxLen;
    float voxelSize;
};

#include "octreeHelper.frag"


//Usefull constants
#define ZERO_TO_ROUND 1000000.0

#define RAYTRACING_CHUNK_PER_AXIS (RAYTRACING_CHUNK_RANGE * 2 + 1)
#define RAYTRACING_SVO_SIZE (RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS)

#define CENTER_OFFSET ivec3(RAYTRACING_CHUNK_RANGE, RAYTRACING_CHUNK_RANGE, RAYTRACING_CHUNK_RANGE)

layout(std430, binding = 2) buffer SsboOctree {
    int octreeData[];
} ssboOctrees[RAYTRACING_SVO_SIZE];

layout(set = 0, binding = 3) uniform SsboInformations {
    int empty;
} ssboInformations[RAYTRACING_SVO_SIZE];


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

    int tempoValue = 0;

    while ( true ) {

        //This should never happen, the precedent test should always be true when we're at the last depth.
        //TODO: remove this security
        if (currentDepth == maxDepth) return getResult( -1, maxDepth );

        uint indexOffset = depthStep(position, currentLen);
        currentIndex += indexOffset;

        //indicate index of the next depth
        int currentValue = ssboOctrees[nonuniformEXT(ssboIndex)].octreeData[currentIndex];
     
        //If true, we found the value of the given position
        if (currentValue <= 0) return getResult( -currentValue, currentDepth );

        //If value not reached, we go to the next memory chunk
        currentIndex = currentValue;

        ++currentDepth;

    }

}

getResult octreeGetWorld(vec3 worldPosition) {

    //Get local position
    LocalPositionResult localPositionResult = getChunkLocalPosition(worldPosition, maxLen);

    //Convert chunk position to ssbo index
    ivec3 centeredChunkPosition = (localPositionResult.chunkPosition - centerChunkPosition) + CENTER_OFFSET;
    int ssboIndex = centeredChunkPosition.x * (RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS) + centeredChunkPosition.y * RAYTRACING_CHUNK_PER_AXIS + centeredChunkPosition.z;
    
    return octreeGet(ssboIndex, uvec3(floor(localPositionResult.localPosition)));

}

struct octreeRayCastResult {
    int returnCode;
    float traveledDistance;
    int voxel;
    vec3 normal;
    vec3 hitPosition;
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
    // if (voxelAndDepth.voxel != 0) {
    //     voxelAndDepth = octreeGet(ssboIndex, voxelPosition);
    // }

 
    int totalLoop = 0;
    while(voxelAndDepth.voxel == 0) {
        
        ++totalLoop;
        if (totalLoop > 100){
            return octreeRayCastResult(8, traveledDistance, voxelAndDepth.voxel, vec3(0), vec3(0)); 
        }

        float leafSize = DEPTH_TO_LEN[voxelAndDepth.depth];

        //Required time to exit the full voxel along each axis.
        traveledAndIndex = getShortestTraveledDistanceAndIndex(position, direction, directionSign, leafSize);             
        
        //Compute position offset and add it to the current position to find the new current position
        vec3 positionOffset = direction * traveledAndIndex.traveledDistance;

        position += positionOffset;
        position = round(position * ZERO_TO_ROUND) / ZERO_TO_ROUND;

        //Add the traveled distance to the total traveled distance
        traveledDistance += traveledAndIndex.traveledDistance;

        //Compute the new voxel position
        voxelPosition = getIntPosition(position, directionSign);

        if (traveledDistance > maxDistance) {
            return octreeRayCastResult(1, traveledDistance, 0, vec3(0, 0, 0), vec3(0));
        }

        //Test if we went out of bound
        //Note: 2: x >= len, 3: x < 0, 4: y >= len, 5: y < 0, 6: z >= len, 7: z < 0
        for (int i = 0; i < 3; ++i) {
            if (voxelPosition[i] >= int(maxLen)){
                return octreeRayCastResult(2 + i*2, traveledDistance, 0, vec3(0, 0, 0), vec3(0));
            }
            if (voxelPosition[i] < 0) {
                return octreeRayCastResult(2 + i*2 + 1, traveledDistance, 0, vec3(0, 0, 0), vec3(0));
            }
        }

        voxelAndDepth = octreeGet(ssboIndex, voxelPosition);

    }

    //We only arrive here if found voxel is true

    //Compute normal
    vec3 normal = vec3(0.0);
    normal[traveledAndIndex.index] = -directionSign[traveledAndIndex.index];

    return octreeRayCastResult(0, traveledDistance, voxelAndDepth.voxel, normal, voxelPosition);

}

struct WorldRayCastResult {
    int voxel;
    vec3 hitPosition;
    vec3 voxelWorldPosition;
    vec3 normal;
    float dist;
};

//TODO: mettre les bon in
WorldRayCastResult worldCastRay(in vec3 position, vec3 direction, float maxDistance) {
    
    position /= voxelSize;
    maxDistance /= voxelSize;

    vec3 directionSign = step(0.0, direction) * 2.0 - 1.0;

    LocalPositionResult localPositionResult = getChunkLocalPosition(position, maxLen);
    
    //To ensure that we do not start on the limit of a chunk tower the direction vector
    shiftOnLimitChunk(localPositionResult, directionSign);

    //Store result between iterations
    octreeRayCastResult rayCastRes;
    rayCastRes.voxel = 0;

    //Loop here normaly...
    float totalTraveledDistance = 0.0f;

    int count = 0;
    while (totalTraveledDistance < maxDistance) {
        
        ++count;
        if (count > 100) {
            return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        }
        //TODO: Better ?
        ivec3 centeredChunkPosition = (localPositionResult.chunkPosition - centerChunkPosition) + CENTER_OFFSET;

        if (centeredChunkPosition.x < 0) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        if (centeredChunkPosition.y < 0) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        if (centeredChunkPosition.z < 0) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );

        if (centeredChunkPosition.x >= RAYTRACING_CHUNK_PER_AXIS) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        if (centeredChunkPosition.y >= RAYTRACING_CHUNK_PER_AXIS) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );
        if (centeredChunkPosition.z >= RAYTRACING_CHUNK_PER_AXIS) return WorldRayCastResult( 0, vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), totalTraveledDistance * voxelSize );

        int ssboIndex = centeredChunkPosition.x * (RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS) + centeredChunkPosition.y * RAYTRACING_CHUNK_PER_AXIS + centeredChunkPosition.z;
        //Up to here

        vec3 oldLocal = localPositionResult.localPosition;
        
        if (ssboInformations[ssboIndex].empty == 1) {
            
            //TODO: changer le type pour mettre une majuscule
            shortestResult res = getShortestTraveledDistanceAndIndex(localPositionResult.localPosition, direction, directionSign, maxLen);

            vec3 positionOffset = direction * res.traveledDistance;
            localPositionResult.localPosition += positionOffset;
            localPositionResult.localPosition = round(localPositionResult.localPosition * ZERO_TO_ROUND) / ZERO_TO_ROUND;

            if (totalTraveledDistance + res.traveledDistance > maxDistance) rayCastRes.returnCode = 1;
            else rayCastRes.returnCode = 2 + res.index * 2 + int(directionSign[res.index] < 0);

            //TODO: better ?
            rayCastRes.traveledDistance = res.traveledDistance;
            rayCastRes.voxel = 0;


            //TODO: copier la partie !processable de castSVORay dans World.cpp
        }
        else {

            rayCastRes = octreeCastRay(ssboIndex, localPositionResult.localPosition, direction, maxDistance - totalTraveledDistance);

            // break;
        }

        totalTraveledDistance += rayCastRes.traveledDistance;

        if (rayCastRes.returnCode < 2) {
            break;
            //TODO: peut être faire un comportement différent entre 0 et 1 ? (break dans 1 et return dans 2 pour ne pas calculer les positions par exemple ?)
        }
        else {
            
                //Note: We could call shiftOnLimitChunk here instead of using the return
                //TODO: Maybe the problem of ANGLE is here
                ivec2 newChunkOffset = codeToOffset[rayCastRes.returnCode - 2];
                localPositionResult.localPosition[newChunkOffset[0]] = ( (-newChunkOffset[1] + 1) / 2 ) * float(maxLen); //This calcul will transform 1 to 0 and -1 to 1
                localPositionResult.chunkPosition[newChunkOffset[0]] += newChunkOffset[1];
            

        }
        
    }

    //We can recalculate it like this !
    vec3 worldPosition = localPositionResult.localPosition + (localPositionResult.chunkPosition * maxLen) - maxLen / 2;
    vec3 voxelWorldPosition = rayCastRes.hitPosition + (localPositionResult.chunkPosition * maxLen) - maxLen / 2;

    //Resultat marant
    //return WorldRayCastResult( rayCastRes.voxel, worldPosition * voxelSize, vec3(0), rayCastRes.normal, totalTraveledDistance * voxelSize );
    return WorldRayCastResult( rayCastRes.voxel, worldPosition * voxelSize, voxelWorldPosition * voxelSize, rayCastRes.normal, totalTraveledDistance * voxelSize );

}
