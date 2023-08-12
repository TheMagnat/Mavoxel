
#pragma once

#include <World/Chunk.hpp>
#include <World/SimpleVoxel.hpp>

#include <glm/vec3.hpp>
#include <array>

namespace mav {

    class Chunk;

    struct RayCollisionInformations {

        RayCollisionInformations(int32_t voxelP, glm::uvec3 const& positionP, std::vector<glm::vec3> const& normalsP, float distanceP, Chunk* parentChunk = nullptr);

        //Information about the voxel owning the face
        int32_t voxel;
        Chunk* chunk;

        glm::uvec3 position;
        std::vector<glm::vec3> normals;

        float distance;

    };

}
