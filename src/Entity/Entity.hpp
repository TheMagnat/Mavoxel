
#pragma once

#include <glm/vec3.hpp>


namespace mav {

    enum class Direction {
        FRONT,
        UP,
        RIGHT
    };

    class Entity {

        public:
            Entity(glm::vec3 const& entityStartPosition, glm::vec3 const& entityFrontVector, glm::vec3 const& entityRightVector)
                : position(entityStartPosition), front(entityFrontVector), right(entityRightVector) {}


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

        public:

            bool freeFlight = true;

            glm::vec3 const& position;
            glm::vec3 const& front;
            glm::vec3 const& right;


            glm::vec3 velocity;
            


    };

}
