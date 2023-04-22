
#pragma once

#include <World/Chunk.hpp>
#include <World/SimpleVoxel.hpp>

#include <glm/vec3.hpp>
#include <array>

namespace mav {

    class Chunk;

    struct CollisionFace {

        CollisionFace(SimpleVoxel* focusedVoxel, Chunk* parrentChunk, std::vector<float> const& vertices, float distanceP)
            : voxel(focusedVoxel), chunk(parrentChunk), distance(distanceP) {
            
            //Data size in vertices
            //TODO: faire en sort d'aller chercher le 10
            for(size_t i = 0, j = 0; i < 4; ++i, j += 10) {
                points[i].x = vertices[j + 0];
                points[i].y = vertices[j + 1];
                points[i].z = vertices[j + 2];
            }

            normal.x = vertices[3 + 0];
            normal.y = vertices[3 + 1];
            normal.z = vertices[3 + 2];

        }

        //Information about the voxel owning the face
        SimpleVoxel* voxel;
        Chunk* chunk;

        std::array<glm::vec3, 4> points;
        glm::vec3 normal;

        float distance;

    };

}
