#pragma once

#include <glm/glm.hpp>

namespace mav {

    class PhysicSystem {

        public:
            PhysicSystem(float gravityStrength);

            void applyGravity(glm::vec3& velocity, float mass, float deltaTime) const;
            void applyAirFriction(glm::vec3& velocity, float mass, float deltaTime) const;
            void applyGroundFriction(glm::vec3& velocity, float mass, float deltaTime) const;

            inline void setGravityStrength(float gravityStrength) { gravityStrength_ = gravityStrength; }

        private:
            float gravityStrength_;

    };

}
