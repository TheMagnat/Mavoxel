
#pragma once

#include <World/Chunk.hpp>
#include <World/SimpleVoxel.hpp>

#include <glm/vec3.hpp>
#include <array>

namespace mav {

    class Chunk;

    struct CollisionFace {

        CollisionFace(SimpleVoxel* focusedVoxel, Chunk* parrentChunk, std::vector<float> const& vertices, float distanceP);

        std::array<glm::vec3, 4> getOffsettedPoints(float offsetValue) const;


        //Information about the voxel owning the face
        SimpleVoxel* voxel;
        Chunk* chunk;

        std::array<glm::vec3, 4> points;
        glm::vec3 normal;

        float distance;

    };

}
