

#include <Collision/CollisionFace.hpp>

namespace mav {

    CollisionFace::CollisionFace(int32_t voxelP, Chunk* parentChunk, glm::uvec3 const& positionP, glm::vec3 const& normalP, float distanceP)
        : voxel(voxelP),  chunk(parentChunk), position(positionP), normal(normalP), distance(distanceP) {}

}
