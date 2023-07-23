
#pragma once

#include <World/Chunk.hpp>
#include <World/SimpleVoxel.hpp>

#include <glm/vec3.hpp>
#include <array>

namespace mav {

    class Chunk;

    struct CollisionFace {

        CollisionFace(int32_t voxelP, Chunk* parentChunk, glm::uvec3 const& positionP, glm::vec3 const& normalP, float distanceP);

        //Information about the voxel owning the face
        int32_t voxel;
        Chunk* chunk;

        glm::uvec3 position;
        glm::vec3 normal;

        float distance;

    };

}
