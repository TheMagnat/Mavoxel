
#include <Entity/Entity.hpp>

#ifndef NDEBUG
#include <Core/DebugGlobal.hpp>
#endif

#define FREE_FLIGHT_MULTIPLIER 5.0f

//TODO: le metre en paramètre de la class ?
const static float friction = 0.01;
const static float aerialFriction = 0.75;
const static float minimumLength = 0.001;
const static float dampingFactor = 0.5;

namespace mav {

    //TODO: peut être avoir un bool qui vérifie qu'on peut utiliser front et right ?
    Entity::Entity(AABB boundingBox) : velocity(0.0f), front_(nullptr), right_(nullptr), boundingBox_(boundingBox), gravity_(nullptr)
    #ifndef NDEBUG
        , entityBox(&DebugGlobal::debugEnvironment, {
            {0.1f, 0.1f, 0.1f},
            {0.5f, 0.5f, 0.5f},
            {1.0f, 1.0f, 1.0f}
        //TODO: rendre la taille du voxel de debug plus paramétrable (prendre un glm::vec3)
        }, 0.5f * 0.95f, boundingBox_.center )
    #endif
    {

        #ifndef NDEBUG
            entityBox.initialize();
            entityBox.setColor(glm::vec3(1.0f, 0.0f, 0.0f));
        #endif

    }

    void Entity::setGravity(const Gravity* newGravity) {
        gravity_ = newGravity;
    }


    //Movement logic
    void Entity::addVelocity(Direction direction, float strength, float deltaTime){
        
        switch(direction) {
            case Direction::FRONT:
                if(!freeFlight) {
                    glm::vec3 trueFront = glm::normalize(glm::vec3(front_->x, 0.0f, front_->z));
                    velocity += trueFront * (strength * deltaTime);
                }
                else {
                    velocity += (*front_) * (strength * FREE_FLIGHT_MULTIPLIER * deltaTime);
                }
                break;

            case Direction::UP:
                velocity.y += strength * deltaTime;
                break;

            case Direction::RIGHT:
                if(!freeFlight) {
                    glm::vec3 trueRight = glm::normalize(glm::vec3(right_->x, 0.0f, right_->z));
                    velocity += trueRight * (strength * deltaTime);
                }
                else {
                    velocity += (*right_) * (strength * deltaTime);
                }
                break;
        }

    }
    
    /**
     * Direction: binary representation of the direction to apply the strength :
     *      001 -> Front
     *      010 -> Right
     *      100 -> Up
    */
    void Entity::addVelocity(glm::vec3 const& direction, float strength, float deltaTime){

        glm::vec3 normalizedDirectionStrength = glm::normalize(direction) * strength;
        
        if(direction.x != 0) {

            if(!freeFlight) {
                glm::vec3 trueRight = glm::normalize(glm::vec3(right_->x, 0.0f, right_->z));
                velocity += trueRight * (normalizedDirectionStrength.x * deltaTime);
            }
            else {
                velocity += (*right_) * (normalizedDirectionStrength.x * FREE_FLIGHT_MULTIPLIER * deltaTime);
            }

        }
        if(direction.y != 0) {

            velocity.y += normalizedDirectionStrength.y * deltaTime;

        }
        if(direction.z != 0) {

            if(!freeFlight) {
                glm::vec3 trueFront = glm::normalize(glm::vec3(front_->x, 0.0f, front_->z));
                velocity += trueFront * (normalizedDirectionStrength.z * deltaTime);
            }
            else {
                velocity += (*front_) * (normalizedDirectionStrength.z * FREE_FLIGHT_MULTIPLIER * deltaTime);
            }

        }

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

        #ifndef NDEBUG
            entityBox.setPosition(boundingBox_.center);
        #endif

    }

    bool Entity::update(float elapsedTime, World const& world) {

        timeSinceLastJump += elapsedTime;
        
        //If we're not in free flight we apply gravity
        if (!freeFlight)
            //TODO: Make the gravity more smouth. Actuellement les entités tombent trop vite vers le bas (même après un saut)
            if (gravity_) gravity_->apply(velocity, elapsedTime);


        //We apply friction and damping factor on our velocity
        float timeGroundFriction = pow(friction, elapsedTime);
        float timeAerialFriction = pow(aerialFriction, elapsedTime);
        if (freeFlight) {
            velocity *= timeGroundFriction;
        }
        else {
            velocity.x *= timeGroundFriction;
            velocity.z *= timeGroundFriction;
            velocity.y *= timeAerialFriction;
        }

        if( glm::length(velocity) < minimumLength ) velocity = glm::vec3(0.0f);

        //And we calculate the new position
        bool positionUpdated = false;
        if( glm::length(velocity) ) {

            auto [collisionVelocity, encounteredCollision] = world.collide(boundingBox_, velocity * elapsedTime);

            //On the collisions, put velocity at 0
            if (encounteredCollision.y != 0) velocity.y = 0.0f;
            if (encounteredCollision.x != 0) velocity.x = 0.0f;
            if (encounteredCollision.z != 0) velocity.z = 0.0f;

            //Here we hitted the ground
            if (encounteredCollision.y == -1) {
                groundHit();
            }
            else {
                if (onTheGround) groundLeft();
            }

            //True velocity recalculated
            // velocity = collisionVelocity / elapsedTime;
        
            //camera_.ProcessKeyboard(collisionVelocity);

            //Good collision
            boundingBox_.center += collisionVelocity;

            //No collision
            // boundingBox_.center += velocity * elapsedTime;

            #ifndef NDEBUG
                entityBox.setPosition(boundingBox_.center);
            #endif

            positionUpdated = true;

        }

        return positionUpdated;

    }
    
    void Entity::draw(VkCommandBuffer commandBuffer) const {
        #ifndef NDEBUG
            entityBox.draw(commandBuffer);
        #endif
    }

}
