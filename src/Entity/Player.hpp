
#pragma once

#include <GLObject/Camera.hpp>
#include <Entity/Entity.hpp>

namespace mav {

    //TODO: Passer dans le .cpp (pareil pour Entity.hpp)
    class Player : public Entity {

            public:
                Player(glm::vec3 const& playerStartPosition, float voxelSize)
                    : Entity( AABB(playerStartPosition, glm::vec3(voxelSize/2.0f * 0.95f, voxelSize * 0.95f, voxelSize/2.0f * 0.95f)) ),
                    camera_(playerStartPosition), voxelSize_(voxelSize)
                {
                    front_ = &camera_.Front;
                    right_ = &camera_.Right;
                    // box.updatePos(position.x, position.y - voxelSize/2.0f, position.z);
                }

                void update(float elapsedTime) {
                                        
                    camera_.ProcessKeyboard(velocity, elapsedTime);

                    //Update collision box
                    // box.updatePos(position);

                    #ifndef NDEBUG
                        entityBox.setPosition(boundingBox_.center);
                    #endif

                    float toMult = pow(friction, elapsedTime);
                    velocity *= toMult;

                }

                bool update(float elapsedTime, World const& world) {
                    
                    bool positionUpdated = Entity::update(elapsedTime, world);
                    
                    // Update camera position
                    if (positionUpdated) {
                        //TODO: un truc plus clean ?
                        camera_.Position = boundingBox_.center;
                        camera_.Position.y += voxelSize_ * 0.95 / 2.0f;
                        camera_.updateFrustum();
                    }

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

                //World data
                float voxelSize_;

    };

}
