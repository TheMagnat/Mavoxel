#include <Collision/AABB.hpp>


AABB::AABB(glm::vec3 pos, float size)
    : minCoords(pos - (size/2.0f)), maxCoords(pos + (size/2.0f)) {}