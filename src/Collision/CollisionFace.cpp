

#include <Collision/CollisionFace.hpp>

namespace mav {

    RayCollisionInformations::RayCollisionInformations(int32_t voxelP, glm::uvec3 const& positionP, std::vector<glm::vec3> const& normalsP, float distanceP, Chunk* parentChunk)
        : voxel(voxelP),  chunk(parentChunk), position(positionP), normals(normalsP), distance(distanceP) {}

}
