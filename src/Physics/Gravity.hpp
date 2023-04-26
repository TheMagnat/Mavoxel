#pragma once

#include <glm/vec3.hpp>

namespace mav {

    class Gravity {

        public:
            Gravity(float strength);

            void apply(glm::vec3& velocity, float deltaTime) const;

        private:
            float strength_;

    };

}
