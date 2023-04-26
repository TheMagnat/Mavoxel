#include "Gravity.hpp"

namespace mav {

    Gravity::Gravity(float strength) : strength_(strength) {}

    void Gravity::apply(glm::vec3& velocity, float deltaTime) const {
    
        velocity.y -= strength_ * deltaTime;
    
    }

}
