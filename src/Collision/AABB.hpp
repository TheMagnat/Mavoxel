
#pragma once

#include <glm/vec3.hpp>
#include <glm/gtx/scalar_relational.hpp>

//AABB = Axis-aligned bounding boxes
struct AABB {

    // float minX, maxX;
    // float minY, maxY;
    // float minZ, maxZ;

    glm::vec3 minCoords, maxCoords;

    AABB(glm::vec3 pos, float size);

};