#include <Collision/AABB.hpp>

namespace mav {

    AABB::AABB(glm::vec3 const& pos, float size)
        : center(pos), extents(size/2.0f) {}

    AABB::AABB(glm::vec3 const& pos, glm::vec3 const& extents)
        : center(pos), extents(extents) {}

    void AABB::updatePos(glm::vec3 const& pos) {
        center = pos;
    }

    void AABB::updatePos(float x, float y, float z) {
        center.x = x;
        center.y = y;
        center.z = z;
    }

}
