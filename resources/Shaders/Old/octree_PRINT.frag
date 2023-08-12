#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require

#include "octreeHelper.frag"

//TODO: Add uniform with world informations
layout (set = 0, binding = 1) uniform WorldOctreeInformations {
    ivec3 centerChunkPosition;
    int maxDepth;
    int maxLen;
    float voxelSize;
};

//Octree info (need uniforms...)
#define MAX_DEPTH 4
const float depthToLen_[MAX_DEPTH] = float[MAX_DEPTH](8, 4, 2, 1);

#define RAYTRACING_CHUNK_RANGE 1
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
    

    ivec3 lolpos = ivec3(position);
    ivec3 test = ivec3(position);
    test -= ivec3(mask) * int(currentLen);

    // if (TexPos.x < 0.5) {
    //     if (test.x < 0 || test.y < 0 || test.z < 0) {
            
    //         debugPrintfEXT("Chelou: %i %i %i\nid: %i\n", lolpos.x, lolpos.y, lolpos.z, id);
    //     }
    //     if (test.x >= maxLen || test.y >= maxLen || test.z >= maxLen) {
            
    //         debugPrintfEXT("Chelou 2: %i %i %i\n", lolpos.x, lolpos.y, lolpos.z);
    //     }
    // }
    
    
    position -= mask * currentLen;

    return uint(dot(mask, uvec3(1, 2, 4)));

}

struct getResult {
    int voxel;
    uint depth;
};

getResult octreeGet(int ssboIndex, uvec3 position) {

    int secondIndex = ssboIndex;
    uint thordIndex = ssboIndex;
    int doubledIndex = ssboIndex*2;
    
    // if (position.x > 32 || position.y > 32 || position.z > 32) {
        
    //     debugPrintfEXT("Chelou: %u %u %u\n", position.x, position.y, position.z);
    // }

    if (CENTER){
        debugPrintfEXT("wwwwwwwwwww GET wwwwwwwwwww\n");
        debugPrintfEXT("Params: ssboIndex: %i, len: %u, position: %v3u\n", ssboIndex, maxLen, position);
        debugPrintfEXT("SSBO CONTENT: 0=%i, 1=%i, 2=%i, 3=%i, 4=%i, 5=%i, 6=%i, 7=%i\n", ssboOctrees[ssboIndex].octreeData[0], ssboOctrees[ssboIndex].octreeData[1], ssboOctrees[ssboIndex].octreeData[2], ssboOctrees[ssboIndex].octreeData[3], ssboOctrees[ssboIndex].octreeData[4], ssboOctrees[ssboIndex].octreeData[5], ssboOctrees[ssboIndex].octreeData[6], ssboOctrees[ssboIndex].octreeData[7]);
    }


    uvec3 copyPosition = position;

    uint currentIndex = 0;
    uint currentDepth = 0;
    uint currentLen = maxLen;

    int tempoValue = 0;

    while ( true ) {

        //This should never happen, the precedent test should always be true when we're at the last depth.
        //TODO: remove this security
        if (currentDepth == maxDepth) {
            if (CENTER){
                debugPrintfEXT("Max depth...\n");
                debugPrintfEXT("wwwwwwwwwww BAD END GET wwwwwwwwwww\n");
            }
            return getResult( -1, maxDepth );
        }

        uint indexOffset = depthStep(copyPosition, currentLen);
        currentIndex += indexOffset;

        if (CENTER){
            debugPrintfEXT("Step done, values: currentLen: %u, currentIndex: %u, position: %v3u\n", currentLen, currentIndex, copyPosition);
            debugPrintfEXT("SSBO index: %i\n", ssboIndex);
        }

        //indicate index of the next depth
        int currentValue = ssboOctrees[nonuniformEXT(ssboIndex)].octreeData[currentIndex];
        // int currentValue2 = ssboOctrees[ssboIndex].octreeData[currentIndex];
        // int currentValueInt = ssboOctrees[ssboIndex].octreeData[int(currentIndex)];
        // int currentValueSeven = ssboOctrees[ssboIndex].octreeData[7];
        // tempoValue = ssboOctrees[uint(ssboIndex)].octreeData[currentIndex];
        
        // int test1 = ssboOctrees[secondIndex].octreeData[7];
        // int test2 = ssboOctrees[thordIndex].octreeData[7];
        // int test3 = ssboOctrees[doubledIndex/2].octreeData[7];
        // if (CENTER) {
            
        //     if (ssboIndex == 12) {
        //         debugPrintfEXT("ssboIndex value IS 12\n");
        //     }

        // }

        // int currentValue = 0;
        // for (uint i = 0; i < RAYTRACING_SVO_SIZE; ++i) {
        //     if (i == ssboIndex) currentValue = ssboOctrees[i].octreeData[currentIndex];
        // }

        // int loltry = ssboOctrees[12].octreeData[currentIndex];

        // if (CENTER){
        //     debugPrintfEXT("More info: current Index as uint: %u and int: %i\n Offset: %u\n", uint(currentIndex), int(currentIndex), indexOffset);
        //     debugPrintfEXT("Value: %i, value no 7: %i\n", ssboOctrees[ssboIndex].octreeData[currentIndex], ssboOctrees[ssboIndex].octreeData[7]);

        //     debugPrintfEXT("currentValue: %i, tempoValue: %i\ncurrentValue2: %i, currentValueInt: %i, currentValueSeven: %i\n", currentValue, tempoValue, currentValue2, currentValueInt, currentValueSeven);
        //     debugPrintfEXT("LoL last try: %i\n", loltry);
        //     debugPrintfEXT("Chelou try: test1 %i test2 %i test3 %i\n", test1, test2, test3);
        //     debugPrintfEXT("LOOP VALUE: %i\n", loopValue);
            
        // }

        if (CENTER){
            debugPrintfEXT("Current value (new index ?): %i\n", currentValue);
        }

        //If true, we found the value of the given position
        if (currentValue <= 0) {
            if (CENTER){
                debugPrintfEXT("wwwwwwwwwww END GET wwwwwwwwwww\n");
            }
            return getResult( -currentValue, currentDepth );
        }

        //If value not reached, we go to the next memory chunk
        currentIndex = currentValue;

        ++currentDepth;

    }

}

getResult DEBUGoctreeGet(int ssboIndex, uvec3 position) {
    
    uint currentIndex = 0;
    uint currentDepth = 0;
    uint currentLen = maxLen;

    debugPrintfEXT("SSBO index: %i %u, length: %i, depth: %i %i\n", ssboIndex, currentLen, ssboOctrees[ssboIndex].octreeData.length(), currentDepth, maxDepth);

    while ( true ) {

        //This should never happen, the precedent test should always be true when we're at the last depth.
        //TODO: remove this security
        if (currentDepth == maxDepth) return getResult( -1, maxDepth );

        uint indexOffset = depthStep(position, currentLen);
        currentIndex += indexOffset;

        //indicate index of the next depth
        int currentValue = ssboOctrees[ssboIndex].octreeData[currentIndex]; //TODO: trouver un moyen d'avoir plusieurs octreeData et choisir le bon en parametre de la fonction

        debugPrintfEXT("Current: %i - %u (%i). Len: %u, index: %u\n", currentValue, currentDepth, ssboOctrees[ssboIndex].octreeData[currentIndex], currentLen, currentIndex);

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
    

    //direction = round(direction * 1000000.0) / 1000000.0;
    //TESTDEBUG

    //Positive or negative direction
    vec3 directionSign = step(0.0, direction) * 2.0 - 1.0;

    //Index of the first voxel
    ivec3 voxelPosition = getIntPosition(position, directionSign);

    getResult voxelAndDepth = getResult(0, 0);
    float traveledDistance = 0.0f;

    shortestResult traveledAndIndex;
    
    // if (voxelPosition.x < 0 || voxelPosition.y < 0 || voxelPosition.z < 0) {
    //     debugPrintfEXT("Voxel position: %i %i %i\nPosition: %f %f %f\nDirection sign: %f %f %f\n\n", voxelPosition.x, voxelPosition.y, voxelPosition.z, position.x, position.y, position.z, directionSign.x, directionSign.y, directionSign.z);
    // }

    if (CENTER){
        debugPrintfEXT("::::::::::::::::: START CAST RAY :::::::::::::::::\n");
        debugPrintfEXT("Params:\n ssboIndex: %i, maxDistance: %f\n position: %v3f\n direction: %v3f\n direction sign: %v3f\n Voxel start position: %v3i\n", ssboIndex, maxDistance, position, direction, directionSign, voxelPosition);
    }
    
    voxelAndDepth = octreeGet(ssboIndex, voxelPosition);
    if (CENTER){
        debugPrintfEXT("SSBO CONTENT (first get): 0=%i, 1=%i, 2=%i, 3=%i, 4=%i, 5=%i, 6=%i, 7=%i\n", ssboOctrees[ssboIndex].octreeData[0], ssboOctrees[ssboIndex].octreeData[1], ssboOctrees[ssboIndex].octreeData[2], ssboOctrees[ssboIndex].octreeData[3], ssboOctrees[ssboIndex].octreeData[4], ssboOctrees[ssboIndex].octreeData[5], ssboOctrees[ssboIndex].octreeData[6], ssboOctrees[ssboIndex].octreeData[7]);
    }
    // voxelAndDepth.voxel = 0;
    if (voxelAndDepth.voxel != 0) {

        if (CENTER){
            debugPrintfEXT("Needed second get because first: voxel=%i, depth=%u\n", voxelAndDepth.voxel, voxelAndDepth.depth);
        }
        voxelAndDepth = octreeGet(ssboIndex, voxelPosition);
    }

    if (CENTER){
        debugPrintfEXT("First get: voxel=%i, depth=%u\n", voxelAndDepth.voxel, voxelAndDepth.depth);
    }
 
    int totalLoop = 0;
    while(voxelAndDepth.voxel == 0) {

        if (CENTER){
            debugPrintfEXT("-------------\nOctree ray cast loop iteration nb: %i\n", totalLoop);
        }
        
        ++totalLoop;
        if (totalLoop > 100){
            if (CENTER){
                debugPrintfEXT("Total loop > 100...\n");
            }
            return octreeRayCastResult(8, traveledDistance, voxelAndDepth.voxel, vec3(0)); 
        }

        float leafSize = depthToLen_[voxelAndDepth.depth];

        //Required time to exit the full voxel along each axis.
        traveledAndIndex = getShortestTraveledDistanceAndIndex(position, direction, directionSign, leafSize);             
        
        //Compute position offset and add it to the current position to find the new current position
        vec3 positionOffset = direction * traveledAndIndex.traveledDistance;

        if (CENTER){
            debugPrintfEXT("Leaf size: %f, traveled distance: %f\n Old position: %v3f\n Offset: %v3f\n", leafSize, traveledAndIndex.traveledDistance, position, positionOffset);
        }

        position += positionOffset;
        position = round(position * 10000000.0) / 10000000.0;
        
        if (CENTER){
            debugPrintfEXT("New position (with round): %v3f\n", position);
        }

        //Add the traveled distance to the total traveled distance
        traveledDistance += traveledAndIndex.traveledDistance;

        if (CENTER){
            debugPrintfEXT("total traveled: %f\n", traveledDistance);
        }

        //Compute the new voxel position
        voxelPosition = getIntPosition(position, directionSign);


        if (CENTER){
            debugPrintfEXT("New voxel position: %v3i\n", voxelPosition);
        }


        if (traveledDistance > maxDistance) {
            if (CENTER){
                debugPrintfEXT("traveledDistance > maxDistance\n");
            }
            return octreeRayCastResult(1, traveledDistance, 0, vec3(0, 0, 0));
        }

        //Test if we went out of bound
        //Note: 2: x >= len, 3: x < 0, 4: y >= len, 5: y < 0, 6: z >= len, 7: z < 0
        for (int i = 0; i < 3; ++i) {
            if (voxelPosition[i] >= int(maxLen)){
                if (CENTER){
                    debugPrintfEXT("Ouf of bound: %i\n", voxelPosition[i]);
                }
                return octreeRayCastResult(2 + i*2, traveledDistance, 0, vec3(0, 0, 0));
            }
            if (voxelPosition[i] < 0) {
                if (CENTER){
                    debugPrintfEXT("Ouf of bound: %i\n", voxelPosition[i]);
                }
                return octreeRayCastResult(2 + i*2 + 1, traveledDistance, 0, vec3(0, 0, 0));
            }
        }

        voxelAndDepth = octreeGet(ssboIndex, voxelPosition);
        if (CENTER){
            debugPrintfEXT("New get: voxel=%i, depth=%u\n", voxelAndDepth.voxel, voxelAndDepth.depth);
            debugPrintfEXT("SSBO CONTENT (end get): 0=%i, 1=%i, 2=%i, 3=%i, 4=%i, 5=%i, 6=%i, 7=%i\n", ssboOctrees[ssboIndex].octreeData[0], ssboOctrees[ssboIndex].octreeData[1], ssboOctrees[ssboIndex].octreeData[2], ssboOctrees[ssboIndex].octreeData[3], ssboOctrees[ssboIndex].octreeData[4], ssboOctrees[ssboIndex].octreeData[5], ssboOctrees[ssboIndex].octreeData[6], ssboOctrees[ssboIndex].octreeData[7]);
        }

    }

    //We only arrive here if found voxel is true

    //Compute normal
    vec3 normal = vec3(0.0);
    normal[traveledAndIndex.index] = -directionSign[traveledAndIndex.index];

    if (CENTER){
        debugPrintfEXT("::::::::::::::::: NORMAL END CAST RAY :::::::::::::::::\n");
    }

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

    if (CENTER){
        debugPrintfEXT("World cast ray starting, params:\n position = %v3f\n direction = %v3f\n directionSign = %v3f\n----------\n", position, direction, directionSign);
    }

    LocalPositionResult localPositionResult = getChunkLocalPosition(position, maxLen, 1.0);

    if (CENTER){
        debugPrintfEXT("Local position: %v3f\n Chunk position = %v3i\n----------\n", localPositionResult.localPosition, localPositionResult.chunkPosition);
    }
    //Store result between iterations
    octreeRayCastResult rayCastRes;
    rayCastRes.voxel = 0;

    //Loop here normaly...
    float totalTraveledDistance = 0.0f;

    //debug
    int maxIter = 10;
    int counter = 0;

    if (CENTER){
        debugPrintfEXT("============= LOOP START =============\n");
    }

    while (totalTraveledDistance < maxDistance) {

        if (CENTER){
            debugPrintfEXT("Loop num: %i, actual distance = %f (max = %f)\n", counter, totalTraveledDistance, maxDistance);
        }
        
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

        if (CENTER){
            debugPrintfEXT("Centerred chunk position: %v3i (SSBO index = %i)\n", centeredChunkPosition, ssboIndex);
        }

        vec3 oldLocal = localPositionResult.localPosition;
        
        if (ssboInformations[ssboIndex].empty == 1) {

            if (CENTER){
                debugPrintfEXT("SSBO is empty...\n");
            }
            
            //TODO: changer le type pour mettre une majuscule
            shortestResult res = getShortestTraveledDistanceAndIndex(localPositionResult.localPosition, direction, directionSign, maxLen);

            vec3 positionOffset = direction * res.traveledDistance;
            localPositionResult.localPosition += positionOffset;
            localPositionResult.localPosition = round(localPositionResult.localPosition * 10000000.0) / 10000000.0;


            if (totalTraveledDistance + res.traveledDistance > maxDistance) rayCastRes.returnCode = 1;
            else rayCastRes.returnCode = 2 + res.index * 2 + int(directionSign[res.index] < 0);

            //TODO: better ?
            rayCastRes.traveledDistance = res.traveledDistance;
            rayCastRes.voxel = 0;


            //TODO: copier la partie !processable de castSVORay dans World.cpp
        }
        else {


            // if ((localPositionResult.localPosition.x == 0 && directionSign.x < 0) || (localPositionResult.localPosition.y == 0 && directionSign.y < 0) || (localPositionResult.localPosition.z == 0 && directionSign.z < 0)) {
            //     debugPrintfEXT("Counter: %i\nPosition: %f %f %f\nDirection sign: %f %f %f\n\n", counter, localPositionResult.localPosition.x, localPositionResult.localPosition.y, localPositionResult.localPosition.z, directionSign.x, directionSign.y, directionSign.z);
            // }

            if (CENTER){
                debugPrintfEXT("Casting ray. Params:\n Local position: %v3f\n direction = %v3f\n max dist = %f\n", localPositionResult.localPosition, direction, (maxDistance - totalTraveledDistance));
            }

            int returnCode;
            float traveledDistance;
            int voxel;
            vec3 normal;

            rayCastRes = octreeCastRay(ssboIndex, localPositionResult.localPosition, direction, maxDistance - totalTraveledDistance);
            
            if (CENTER){
                debugPrintfEXT("------\nRay casted: result:\n Return Code: %i, Voxel: %i, traveled: %f\n normal = %v3f\n", rayCastRes.returnCode, rayCastRes.voxel, rayCastRes.traveledDistance, rayCastRes.normal);
            }

            if (rayCastRes.returnCode == 8) {
                rayCastRes.voxel = 4;
                break;
            }
            else if (rayCastRes.returnCode == 9) {
                rayCastRes.voxel = 5;
                break;
            }
            else if (rayCastRes.returnCode == 10) {
                rayCastRes.voxel = 6;
                break;
            }
            else if (rayCastRes.returnCode == 11) {
                rayCastRes.voxel = 7;
                break;
            }
            // break;
        }

        // return WorldRayCastResult( 6, vec3(0,0,0), vec3(0,0,0), 0);

        totalTraveledDistance += rayCastRes.traveledDistance;

        if (rayCastRes.returnCode < 2) {
            if (CENTER){
                debugPrintfEXT("End of execution because of return code = %i\n", rayCastRes.returnCode);
            }
            break;
            //TODO: peut être faire un comportement différent entre 0 et 1 ? (break dans 1 et return dans 2 pour ne pas calculer les positions par exemple ?)
        }
        else {

            ivec2 codeToOffset[6] = ivec2[6]( ivec2(0, 1), ivec2(0, -1), ivec2(1, 1), ivec2(1, -1), ivec2(2, 1), ivec2(2, -1) );

            // if (TexPos.x < 0.5) {

            vec3 beforeMove = localPositionResult.localPosition;
            //DEBUG/TEST
            int totalMod = 0;
            for (int i = 0; i < 3; ++i) {

                int limitIndex = 0;
                if (localPositionResult.localPosition[i] >= maxLen && directionSign[i] > 0){
                    limitIndex = i*2;
                }
                else if (localPositionResult.localPosition[i] <= 0 && directionSign[i] < 0) {
                    limitIndex = i*2 + 1;
                }
                else {
                    continue;
                }

                ivec2 newChunkOffset = codeToOffset[limitIndex];
                localPositionResult.localPosition[newChunkOffset[0]] = ( (-newChunkOffset[1] + 1) / 2 ) * float(maxLen); //This calcul will transform 1 to 0 and -1 to 1
                localPositionResult.chunkPosition[newChunkOffset[0]] += newChunkOffset[1];
                localPositionResult.worldPositionOffset[newChunkOffset[0]] -= newChunkOffset[1] * float(maxLen);

                totalMod++;

            }

            vec3 traveledTotal = direction * rayCastRes.traveledDistance;
            if (CENTER){
                debugPrintfEXT("Moved to next chunk:\nMod: %i, Ray dist: %f\nNew Position: %v3f\nDirection sign: %v3f\nBefore Mod: %v3f\nOld pos: %v3f\nDiff: %v3f\n\n", totalMod, rayCastRes.traveledDistance, localPositionResult.localPosition, directionSign, beforeMove, oldLocal, traveledTotal);
            }

            // }
            /*
            else {

                //TODO: le mettre en global constante

                ivec2 newChunkOffset = codeToOffset[rayCastRes.returnCode - 2];
                localPositionResult.localPosition[newChunkOffset[0]] = ( (-newChunkOffset[1] + 1) / 2 ) * float(maxLen); //This calcul will transform 1 to 0 and -1 to 1
                localPositionResult.chunkPosition[newChunkOffset[0]] += newChunkOffset[1];
                localPositionResult.worldPositionOffset[newChunkOffset[0]] -= newChunkOffset[1] * float(maxLen);

                // if ((localPositionResult.localPosition.x == 0 && directionSign.x < 0) || (localPositionResult.localPosition.y == 0 && directionSign.y < 0) || (localPositionResult.localPosition.z == 0 && directionSign.z < 0)) {
                //     debugPrintfEXT("Counter: %i\nEdited index: %i, direction: %i\nPosition: %f %f %f\nDirection sign: %f %f %f\n\n", counter, newChunkOffset[0], newChunkOffset[1], localPositionResult.localPosition.x, localPositionResult.localPosition.y, localPositionResult.localPosition.z, directionSign.x, directionSign.y, directionSign.z);
                // }

                //PB: Plusieurs élements arrivent au bout en même temps mais la fonction pour calculer la voxel position le prend pas en compte et avec une pos de 0 et une direction négative ça donne une position unsigned de -1...
                // vec3 traveledTotal = direction * rayCastRes.traveledDistance;
                // if ((localPositionResult.localPosition.x == 0 && directionSign.x < 0) || (localPositionResult.localPosition.y == 0 && directionSign.y < 0) || (localPositionResult.localPosition.z == 0 && directionSign.z < 0)) {
                //     debugPrintfEXT("Counter: %i\nPosition: %f %f %f\nDirection sign: %f %f %f\nOld pos: %f %f %f\nDiff: %f %f %f\n\n", counter, localPositionResult.localPosition.x, localPositionResult.localPosition.y, localPositionResult.localPosition.z, directionSign.x, directionSign.y, directionSign.z, oldLocal.x, oldLocal.y, oldLocal.z, traveledTotal.x, traveledTotal.y, traveledTotal.z);
                // }

            }
            */
            

        }

        counter++;
        
    }


    if (CENTER){
        debugPrintfEXT("============= NORMAL LOOP END =============\n");
    }


    return WorldRayCastResult( rayCastRes.voxel, (localPositionResult.localPosition - localPositionResult.worldPositionOffset) * voxelSize, rayCastRes.normal, totalTraveledDistance * voxelSize );

}
