
#pragma once

#include <GLObject/Camera.hpp>
#include <Entity/Entity.hpp>
#include <World/World.hpp>

//TODO: remove
//DEBUG
#include <iomanip>

//TODO: le metre en paramètre de la class ?
const static float friction = 0.001;
const static float minimumLength = 0.1;

namespace mav {

    //TODO: Passer dans le .cpp (pareil pour Entity.hpp)
    class Player : public Entity {

            public:
                Player(glm::vec3 const& playerStartPosition)
                    : camera_(playerStartPosition), Entity(camera_.Position, camera_.Front, camera_.Right)
                {
                    box.updatePos(position);
                }

                void update(float elapsedTime) {
                                        
                    camera_.ProcessKeyboard(velocity, elapsedTime);

                    //Update collision box
                    box.updatePos(position);

                    #ifndef NDEBUG
                        entityBox.setPosition(position);
                    #endif

                    float toMult = pow(friction, elapsedTime);
                    velocity *= toMult;

                }

                void update(float elapsedTime, World const& world) {
                    
                    if( glm::length(velocity) ) {
                        
                        glm::vec3 collisionVelocity = world.castRay(box, velocity * elapsedTime);
                                                
                        camera_.ProcessKeyboard(collisionVelocity);
                        box.updatePos(position);

                        #ifndef NDEBUG
                            entityBox.setPosition(position);
                        #endif

                    }

                    float toMult = pow(friction, elapsedTime);
                    velocity *= toMult;

                    if( glm::length(velocity) < minimumLength ) velocity = glm::vec3(0.0f);

                }


                //Camera logic
                Camera* getCamera() {
                    return &camera_;
                }

                void updateCamera(double xPos, double yPos) {

                    if (firstMouse) {
                        lastX = xPos;
                        lastY = yPos;
                        firstMouse = false;
                    }

                    float xOffset = xPos - lastX;
                    float yOffset = lastY - yPos; // inversé car va de bas en haut

                    lastX = xPos;
                    lastY = yPos;

                    camera_.ProcessMouseMovement(xOffset, yOffset);

                }

            private:
                //Camera logic
                Camera camera_;
                float lastX;
                float lastY;
                bool firstMouse = true;

    };

}
