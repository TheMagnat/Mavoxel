#pragma once

#include <glm/vec3.hpp>

namespace mav {

    class Gravity {

        public:
            Gravity(float strength) : strength_(strength) {}


            void apply(glm::vec3& velocity, float deltaTime) const {

                velocity.y -= strength_ * deltaTime;

            }

        private:
            float strength_;

    };

}
