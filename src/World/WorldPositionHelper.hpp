
#pragma once

#include <glm/vec3.hpp>
#include <cmath>

namespace mav {


    inline glm::ivec3 getDiscretePosition(glm::vec3 position, float voxelSize) {
        //TODO: Voir si il faut pas utiliser floor
        return glm::ivec3(position/voxelSize);
    }

    //TODO: le faire avec des glm::vec3
    //Return the chunk coordinates and it's corresponding local position of the given position
    inline std::pair<glm::ivec3, glm::vec3> getChunkLocalPosition(glm::vec3 position, size_t chunkSize, float voxelSize) {

		float xSign = std::signbit(position.x) ? -1 : 1;
		float ySign = std::signbit(position.y) ? -1 : 1;
		float zSign = std::signbit(position.z) ? -1 : 1;

		float trueChunkSize = chunkSize * voxelSize;
		//float chunkLen = trueChunkSize - voxelSize_;

		float halfChunkSize = trueChunkSize / 2.0f;

		int xIndex = (position.x + halfChunkSize * xSign) / trueChunkSize;
		int yIndex = (position.y + halfChunkSize * ySign) / trueChunkSize;
		int zIndex = (position.z + halfChunkSize * zSign) / trueChunkSize;

		//TODO: voir si ça marche avec des positions pas pile sur le cube
		float localX = ((position.x + halfChunkSize) - xIndex * trueChunkSize) / voxelSize;
		float localY = ((position.y + halfChunkSize) - yIndex * trueChunkSize) / voxelSize;
		float localZ = ((position.z + halfChunkSize) - zIndex * trueChunkSize) / voxelSize;

        //TODO: Voir si il faut pas utiliser floor
        return { glm::ivec3(xIndex, yIndex, zIndex), glm::vec3(localX, localY, localZ) };
    }
    
    inline glm::uvec3 getDiscreteUnsignedPosition(glm::vec3 position, float voxelSize) {
        return glm::uvec3(position/voxelSize);
    }

    inline float positiveModulo (float a, float b) { return a >= 0 ? fmod(a, b) : fmod( fmod(a, b) + b, b); }

}
