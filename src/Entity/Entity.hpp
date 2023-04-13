
#pragma once

#include <glm/vec3.hpp>

#include <Collision/AABB.hpp>
#include <Physics/Gravity.hpp>
#include <World/World.hpp>

#include <Mesh/DebugVoxel.hpp>


#define GRAVITY_ON false

//TODO: le metre en paramètre de la class ?
const static float friction = 0.01;
const static float aerialFriction = 0.1;
const static float minimumLength = 0.001;
const static float dampingFactor = 0.5f;

namespace mav {

    enum class Direction {
        FRONT,
        UP,
        RIGHT
    };

    class Entity {

        public:
            Entity(float voxelSize, glm::vec3& entityPosition, glm::vec3 const& entityFrontVector, glm::vec3 const& entityRightVector)
                : velocity(0.0f), position(entityPosition), front(entityFrontVector), right(entityRightVector), box(entityPosition, voxelSize * 0.95f), gravity_(nullptr), voxelSize_(voxelSize)
#ifndef NDEBUG
				, entityBox(&Global::debugShader, &Global::debugEnvironment, {
                    {0.1f, 0.1f, 0.1f},
                    {0.5f, 0.5f, 0.5f},
                    {1.0f, 1.0f, 1.0f}
		        }, voxelSize * 0.95f, entityPosition )
#endif
                {

            #ifndef NDEBUG
				entityBox.initialize(true);
                entityBox.setColor(glm::vec3(1.0f, 0.0f, 0.0f));
    		#endif

                }

                void setGravity(const Gravity* newGravity) {
                    gravity_ = newGravity;
                }


                //Movement logic
                void addVelocity(Direction direction, float strength, float deltaTime){
                    
                    switch(direction) {
                        case Direction::FRONT:
                            if(!freeFlight) {
                                glm::vec3 trueFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
                                velocity += trueFront * (strength * deltaTime);
                            }
                            else {
                                velocity += front * (strength * deltaTime);
                            }
                            break;

                        case Direction::UP:
                            velocity.y += strength * deltaTime;
                            break;

                        case Direction::RIGHT:
                            if(!freeFlight) {
                                glm::vec3 trueRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
                                velocity += trueRight * (strength * deltaTime);
                            }
                            else {
                                velocity += right * (strength * deltaTime);
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
                void addVelocity(glm::vec3 const& direction, float strength, float deltaTime){

                    glm::vec3 normalizedDirectionStrength = glm::normalize(direction) * strength;
                    
                    if(direction.x != 0) {

                        if(!freeFlight) {
                            glm::vec3 trueRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
                            velocity += trueRight * (normalizedDirectionStrength.x * deltaTime);
                        }
                        else {
                            velocity += right * (normalizedDirectionStrength.x * deltaTime);
                        }

                    }
                    if(direction.y != 0) {

                        velocity.y += normalizedDirectionStrength.y * deltaTime;

                    }
                    if(direction.z != 0) {

                        if(!freeFlight) {
                            glm::vec3 trueFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
                            velocity += trueFront * (normalizedDirectionStrength.z * deltaTime);
                        }
                        else {
                            velocity += front * (normalizedDirectionStrength.z * deltaTime);
                        }

                    }

                }

                void jump(float strength) {

                    if (onTheGround) {
                        velocity.y += strength;
                        onTheGround = false;
                    }

                }

                bool update(float elapsedTime, World const& world) {
                    
                    //If we're not in free flight we apply gravity
                    if (!freeFlight)
                        //TODO: Make the gravity more smouth. Actuellement les entités tombent trop vite vers le bas (même après un saut)
                        if (gravity_) gravity_->apply(velocity, elapsedTime);


                    //We apply friction and damping factor on our velocity
                    float timeGroundFriction = pow(friction, elapsedTime);
                    if (freeFlight) {
                        velocity *= timeGroundFriction;
                    }
                    else {
                        velocity.x *= timeGroundFriction;
                        velocity.z *= timeGroundFriction;
                        velocity.y *= 1.0f - dampingFactor * elapsedTime;
                    }

                    if( glm::length(velocity) < minimumLength ) velocity = glm::vec3(0.0f);

                    //And we calculate the new position
                    bool positionUpdated = false;
                    if( glm::length(velocity) ) {

                        auto [collisionVelocity, encounteredCollision] = world.castBoxRay(box, velocity * elapsedTime);

                        //On the collisions, put velocity at 0
                        if (encounteredCollision.y != 0) {
                            if (encounteredCollision.y == -1) onTheGround = true;
                            velocity.y = 0.0f;
                        }

                        if (encounteredCollision.x != 0) velocity.x = 0.0f;
                        if (encounteredCollision.z != 0) velocity.z = 0.0f;

                        //True velocity recalculated
                        //velocity = collisionVelocity / elapsedTime;
                 
                        //camera_.ProcessKeyboard(collisionVelocity);
                        position += collisionVelocity;

                        box.updatePos(position);

                        #ifndef NDEBUG
                            entityBox.setPosition(position);
                        #endif

                        positionUpdated = true;

                    }

                    return positionUpdated;

                }
                
                void draw() const {
                    #ifndef NDEBUG
                        entityBox.draw();
                    #endif
                }

        public:
            glm::vec3 velocity;
            glm::vec3& position;
            bool freeFlight = !GRAVITY_ON;
            bool onTheGround = false;

        protected:

            glm::vec3 const& front;
            glm::vec3 const& right;
            
            //Collision
            AABB box;

            //Physics
            const Gravity* gravity_;

            //World data
            float voxelSize_;


            #ifndef NDEBUG
				DebugVoxel entityBox;
    		#endif


    };

}
