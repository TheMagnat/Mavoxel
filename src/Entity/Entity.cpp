
#include <Entity/Entity.hpp>

#ifndef NDEBUG
#include <Core/DebugGlobal.hpp>
#endif

#define GROUND_LEFT_THRESHOLD 0.001

//TODO: le metre en paramètre de la class ?
const static float minimumLength = 0.001;



namespace mav {

    //TODO: peut être avoir un bool qui vérifie qu'on peut utiliser front et right ?
    Entity::Entity(AABB boundingBox, float massP) : velocity(0.0f), movement(0.0f), acceleration(0.0f), mass(massP), boundingBox_(boundingBox), physicSystem_(nullptr) {}

    void Entity::setPhysicSystem(const PhysicSystem* newPhysicSystem) {
        physicSystem_ = newPhysicSystem;
    }
    
    void Entity::addVelocity(glm::vec3 const& direction, float strength, float deltaTime){

        if (direction == glm::vec3(0.0f)) return;

        glm::vec3 normalizedDirectionStrength = glm::normalize(direction) * strength / mass;
        
        velocity += normalizedDirectionStrength * deltaTime;


    }

    void Entity::addVelocity(glm::vec3 const& direction, float strength, float deltaTime, glm::vec3 const& front, glm::vec3 const& right){

        if (direction == glm::vec3(0.0f)) return;

        glm::vec3 normalizedDirectionStrength = glm::normalize(direction) * strength / mass;
        
        if (direction.z != 0) velocity += front * (normalizedDirectionStrength.z * deltaTime);
        if (direction.x != 0) velocity += right * (normalizedDirectionStrength.x * deltaTime);
        if (direction.y != 0) velocity.y += normalizedDirectionStrength.y * deltaTime;

    }

    void Entity::setAcceleration(glm::vec3 const& forceDirection, float magnitude) {
        acceleration = forceDirection * magnitude / mass;
    }

    void Entity::setAcceleration(glm::vec3 const& forceDirection, float magnitude, glm::vec3 const& front, glm::vec3 const& right) {
        
        if (forceDirection == glm::vec3(0.0f)) return;

        glm::vec3 scaledForce = glm::normalize(forceDirection) * magnitude / mass;

        acceleration = glm::vec3(0.0f);
        acceleration += front * (scaledForce.z);
        acceleration += right * (scaledForce.x);
        acceleration.y += scaledForce.y;

    }

    void Entity::jump(float strength) {

        if (remainingJumps > 0
        && timeSinceLastJump >= timeLimitBetweenJumps)
        {
            velocity.y = strength;
            onTheGround = false;

            --remainingJumps;
            timeSinceLastJump = 0;
        }

    }

    void Entity::groundHit() {
        onTheGround = true;
        remainingJumps = maxJumps;
        timeSinceLastJump = timeLimitBetweenJumps;
    }

    void Entity::groundLeft() {
        onTheGround = false;
        --remainingJumps;
    }

    void Entity::update(float elapsedTime) {
        boundingBox_.center += velocity * elapsedTime;
    }

    bool Entity::update(float elapsedTime, World const& world) {

        timeSinceLastJump += elapsedTime;

        //We apply the acceleration on the movement
        movement += acceleration;
        acceleration = glm::vec3(0.0f);

        if (physicSystem_) physicSystem_->applyGravity(velocity, mass, elapsedTime);
        
        //We compute the total velocity
        glm::vec3 totalVelocity = movement + velocity;

        //We apply the forces
        if (physicSystem_) {
            physicSystem_->applyAirFriction(velocity, mass, elapsedTime);
            physicSystem_->applyGroundFriction(movement, mass, elapsedTime);

            if (onTheGround) {
                physicSystem_->applyGroundFriction(velocity, mass, elapsedTime);
            }
        }

        //TODO: voir si utile, voir si faut pas plutôt utiliser velocity dans le test
        if( glm::length(totalVelocity) < minimumLength ) velocity = glm::vec3(0.0f);

        //And we calculate the new position
        bool positionUpdated = false;
        if( glm::length(totalVelocity) ) {

            auto [collisionVelocity, encounteredCollision] = world.collide(boundingBox_, totalVelocity * elapsedTime);

            glm::vec3 collisionPower = glm::abs(totalVelocity * encounteredCollision);

            //On the collisions, put velocity at 0
            if (encounteredCollision.y != 0) velocity.y = 0.0f;
            if (encounteredCollision.x != 0) velocity.x = 0.0f;
            if (encounteredCollision.z != 0) velocity.z = 0.0f;

            if (length(collisionPower) > 1) {
                std::cout << "Choc de puissance: << " << glm::length(collisionPower) << " (" << collisionPower.x << " " << collisionPower.y << " " << collisionPower.z << ")" << std::endl;
            }

            //Here we hitted the ground
            if (encounteredCollision.y == -1) {
                groundHit();
            }
            else {
                //Note: if threshold is not reached, we don't consider we hit the ground but we don't consider we left it too. We're in an in-between state (Allowing the entity to still jump).
                if (onTheGround && abs(collisionVelocity.y) > GROUND_LEFT_THRESHOLD) groundLeft();
            }

            //True velocity recalculated
            // velocity = collisionVelocity / elapsedTime;
        
            //Good collision
            boundingBox_.center += collisionVelocity;

            //No collision
            // boundingBox_.center += velocity * elapsedTime;

            positionUpdated = true;

        }

        return positionUpdated;

    }

}
