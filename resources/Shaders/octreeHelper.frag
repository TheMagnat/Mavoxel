

struct LocalPositionResult {
    ivec3 chunkPosition;
    vec3 localPosition;
};

//Return the chunk coordinates and it's corresponding local position of the given position
LocalPositionResult getChunkLocalPosition(vec3 position, uint chunkSize) {

    position += chunkSize / 2.0;
    
    ivec3 chunkPosition = ivec3(floor(position / chunkSize));
    vec3 localPosition = mod(position, chunkSize);

    return LocalPositionResult( chunkPosition, localPosition );
}

/**
 * This function will add an offset to the currentLocalPosition parameter if its localPosition is on the side of a chunk
 * (which mean between two chunk) and the direction is going toward the other chunk. This help preventing trying to access
 * to a negative position on the first position along the direction.
 */
void shiftOnLimitChunk(inout LocalPositionResult currentLocalPosition, in vec3 directionSign) {
    
    const vec3 maxLenVec = vec3(maxLen);
    const vec3 zeroVec = vec3(0.0);

    bvec3 isGreaterThanMax = bvec3(uvec3(greaterThanEqual(currentLocalPosition.localPosition, maxLenVec)) * uvec3(greaterThan(directionSign, vec3(0))));
    bvec3 isLessThanMax = bvec3(uvec3(lessThanEqual(currentLocalPosition.localPosition, zeroVec)) * uvec3(lessThan(directionSign, vec3(0))));

    currentLocalPosition.localPosition = currentLocalPosition.localPosition * (uvec3(not(isGreaterThanMax)) * uvec3(not(isLessThanMax))) + maxLenVec * vec3(isLessThanMax);
    currentLocalPosition.chunkPosition += ivec3(isGreaterThanMax) * ivec3(1) + ivec3(isLessThanMax) * ivec3(-1);

}

ivec3 getIntPosition(in vec3 position, in vec3 directionSign) {

    return ivec3( directionSign * floor(directionSign * position) - vec3( lessThan(directionSign, vec3(0)) ) );

}

vec3 computeDistanceToSide(in vec3 position, in vec3 direction, in vec3 directionSign, float leafSize) {

    //This convert position to leaf sized position (by dividing with leaf size) and then calculate it's world discrete position, then put it back to normal sized position (by multiplying by leaf size)
    vec3 nextPosition = (directionSign * floor(directionSign * position / leafSize) + directionSign) * leafSize;
    vec3 distTotal = (nextPosition - position) / direction;

    return distTotal;

}


float getShortestTraveledDistance(in vec3 position, in vec3 direction, in vec3 directionSign, float leafSize) {
    //Required time to exit the full voxel along each axis.
    vec3 tMax = computeDistanceToSide(position, direction, directionSign, leafSize);
    return min( min(tMax.x, tMax.y), tMax.z );
}

struct shortestResult {
    float traveledDistance;
    int index;
};

shortestResult getShortestTraveledDistanceAndIndex(in vec3 position, in vec3 direction, in vec3 directionSign, float leafSize) {
    //Required time to exit the full voxel along each axis.
    vec3 tMax = computeDistanceToSide(position, direction, directionSign, leafSize);
    
    vec3 lowestIndexVec = step(tMax, tMax.yzx);
	lowestIndexVec *= (1.0 - lowestIndexVec.zxy);

    int lowestIndex = int(dot(lowestIndexVec, vec3(0, 1, 2)));

    return shortestResult(tMax[lowestIndex], lowestIndex);
}
