
#pragma once

#include <glm/vec3.hpp>
#include <glm/gtx/scalar_relational.hpp>

namespace mav {

    //AABB = Axis-aligned bounding boxes
    struct AABB {

        glm::vec3 center;
        glm::vec3 extents;


        AABB(glm::vec3 const& pos, float size);
        AABB(glm::vec3 const& pos, glm::vec3 const& extents);
        void updatePos(glm::vec3 const& pos);
        void updatePos(float x, float y, float z);

    private:

        float halfSize_;

    };

}
