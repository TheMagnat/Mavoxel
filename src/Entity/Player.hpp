
#pragma once

#include <GLObject/Camera.hpp>
#include <Entity/Entity.hpp>

namespace mav {

    //TODO: Passer dans le .cpp (pareil pour Entity.hpp)
    class Player : public Entity {

            public:
                Player(glm::vec3 const& playerStartPosition, float voxelSize)
                    : camera_(playerStartPosition), Entity(voxelSize, camera_.Position, camera_.Front, camera_.Right)
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

                bool update(float elapsedTime, World const& world) {
                    
                    bool positionUpdated = Entity::update(elapsedTime, world);
                    if (positionUpdated) camera_.updateFrustum();

                    return positionUpdated;

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
