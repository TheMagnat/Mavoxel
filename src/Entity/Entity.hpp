
#pragma once

#include <glm/vec3.hpp>

#include <Collision/AABB.hpp>

#include <Mesh/DebugVoxel.hpp>

namespace mav {

    enum class Direction {
        FRONT,
        UP,
        RIGHT
    };

    class Entity {

        public:
            Entity(glm::vec3 const& entityStartPosition, glm::vec3 const& entityFrontVector, glm::vec3 const& entityRightVector)
                : position(entityStartPosition), front(entityFrontVector), right(entityRightVector), velocity(0.0f), box(entityStartPosition, 1.0f)
#ifndef NDEBUG
				, entityBox(&Global::debugShader, &Global::debugEnvironment, {
                    {0.1f, 0.1f, 0.1f},
                    {0.5f, 0.5f, 0.5f},
                    {1.0f, 1.0f, 1.0f}
		        }, 1.0f, entityStartPosition )
#endif
                {

            #ifndef NDEBUG
				entityBox.initialize(true);
                entityBox.setColor(glm::vec3(1.0f, 0.0f, 0.0f));
    		#endif

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

                
                void draw() const {
                    #ifndef NDEBUG
                        entityBox.draw();
                    #endif
                }

        public:

            bool freeFlight = true;

            glm::vec3 const& position;
            glm::vec3 const& front;
            glm::vec3 const& right;

            glm::vec3 velocity;
            
            //Collision
            mav::AABB box;


            #ifndef NDEBUG
				DebugVoxel entityBox;
    		#endif


    };

}
