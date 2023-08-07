
#include <Physics/PhysicSystem.hpp>

#define VELOCITY_NULL_THRESHOLD 0.01

//For air friction
const static float airDensity = 1.2f; // kg/m³
const static float dragCoefficient = 0.005f; // A constant you need to define based on entity shape
// const static float aerialFriction = 0.95f;

//For ground friction
const static float groundFriction = 0.0005f;


namespace mav {

    PhysicSystem::PhysicSystem(float gravityStrength) : gravityStrength_(gravityStrength) {}

    void PhysicSystem::applyGravity(glm::vec3& velocity, float mass, float deltaTime) const {
    
        velocity.y -= gravityStrength_ * deltaTime;
    
    }

    void PhysicSystem::applyAirFriction(glm::vec3& velocity, float mass, float deltaTime) const {

        float velocitySquared = std::pow(glm::length(velocity), 2);

        glm::vec3 airFrictionForce = -0.5f * airDensity * dragCoefficient * /* area * */ velocitySquared * velocity;

        velocity += (airFrictionForce / mass) * deltaTime;

        // float timeAerialFriction = pow(aerialFriction, deltaTime);
        // velocity *= timeAerialFriction;

        // if (glm::length(velocity) < 0.01) {
        //     velocity = glm::vec3(0.0f);
        // }

    }

    void PhysicSystem::applyGroundFriction(glm::vec3& velocity, float mass, float deltaTime) const {

        float timeGroundFriction = pow(groundFriction, deltaTime);
        velocity *= timeGroundFriction;

        if (glm::length(velocity) < VELOCITY_NULL_THRESHOLD) velocity = glm::vec3(0.0f);

    }
    

}
