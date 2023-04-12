#include <Collision/AABB.hpp>
#include <iostream>
namespace mav {

    AABB::AABB(glm::vec3 const& pos, float size)
        : halfSize_(size/2.0f), minCoords(pos - size/2.0f), maxCoords(pos + size/2.0f), center(pos), extents(size/2.0f) {
            glm::vec3 test = pos + size/2.0f;
        }


    void AABB::updatePos(glm::vec3 const& pos) {
        minCoords = pos - halfSize_;
        maxCoords = pos + halfSize_;
        center = pos;
    }

}