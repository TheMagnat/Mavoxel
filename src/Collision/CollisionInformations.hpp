
#pragma once

#include <glm/vec3.hpp>


namespace mav {

    struct CollisionInformations {

        glm::ivec3 voxelLocalPosition;
        glm::ivec3 chunkPosition;
        glm::vec3 normal;

    };

}
