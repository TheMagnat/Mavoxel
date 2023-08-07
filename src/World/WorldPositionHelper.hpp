
#pragma once

#include <glm/glm.hpp>
#include <cmath>

namespace mav {

    #define ZERO_TO_ROUND 1000000.0f
    #define COMPARISON_DELTA 0.00001f

    //Round position to a certain limit of zero
    inline void roundPosition(glm::vec3& position) {
        // position = round(position * ZERO_TO_ROUND) / ZERO_TO_ROUND;
    }

    inline glm::ivec3 getDiscretePosition(glm::vec3 position, float voxelSize) {
        //TODO: Voir si il faut pas utiliser floor
        return glm::ivec3(position/voxelSize);
    }

    //TODO: le faire avec des glm::vec3
    //Return the chunk coordinates and it's corresponding local position of the given position
    inline std::pair<glm::ivec3, glm::vec3> getChunkLocalPosition(glm::vec3 position, float chunkSize) {

        glm::ivec3 chunkPosition = glm::ivec3(floor(position / chunkSize));
        glm::vec3 localPosition = glm::mod(position, chunkSize);

        return { chunkPosition, localPosition };
    }

    inline glm::bvec3 vec3Not(glm::bvec3 const toInverse) {
        return glm::bvec3(true, true, true) - toInverse;
    }

    inline void shiftOnLimitChunk(glm::ivec3& chunkPosition, glm::vec3& localPosition, glm::vec3 const& directionSign, float chunkLen) {
    
        static const glm::vec3 maxLenVec = glm::vec3(chunkLen);
        static const glm::vec3 zeroVec = glm::vec3(0.0);

        glm::bvec3 isGreaterThanMax = glm::bvec3(glm::uvec3(glm::greaterThanEqual(localPosition, maxLenVec)) * glm::uvec3(glm::greaterThan(directionSign, glm::vec3(0))));
        glm::bvec3 isLessThanMax = glm::bvec3(glm::uvec3(glm::lessThanEqual(localPosition, zeroVec)) * glm::uvec3(glm::lessThan(directionSign, glm::vec3(0))));

        localPosition = localPosition * glm::vec3(glm::uvec3(vec3Not(isGreaterThanMax)) * glm::uvec3(vec3Not(isLessThanMax))) + maxLenVec * glm::vec3(isLessThanMax);
        chunkPosition += glm::ivec3(isGreaterThanMax) * glm::ivec3(1) + glm::ivec3(isLessThanMax) * glm::ivec3(-1);

    }

    
    inline glm::uvec3 getDiscreteUnsignedPosition(glm::vec3 position, float voxelSize) {
        return glm::uvec3(position/voxelSize);
    }

    inline float positiveModulo (float a, float b) { return a >= 0 ? fmod(a, b) : fmod( fmod(a, b) + b, b); }

}
