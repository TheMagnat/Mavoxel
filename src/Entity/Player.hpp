
#pragma once

#include <Entity/Entity.hpp>
#include <GLObject/Camera.hpp>


namespace mav {

    //TODO: Passer dans le .cpp (pareil pour Entity.hpp)
    class Player : public Entity {

            public:
                Player(glm::vec3 const& playerStartPosition)
                    : camera_(playerStartPosition), Entity(camera_.Position, camera_.Front, camera_.Right)
                {
                    lastX = 0;
                    lastY = 0;
                }

                void update(float deltaTime) {
                    
                    //TODO: le metre en paramètre de la class ?
                    const static float friction = 0.001;

                    // if (velocity.x != 0)
                    //     camera_.ProcessKeyboard(mav::RIGHT, velocity.x, deltaTime);
                    // if (velocity.y != 0)
                    //     camera_.ProcessKeyboard(mav::FORWARD, velocity.y, deltaTime);
                    // if (velocity.z != 0)
                    //     camera_.ProcessKeyboard(mav::FORWARD, velocity.z, deltaTime);
                    
                    camera_.ProcessKeyboard(velocity, deltaTime);

                    float toMult = pow(friction, deltaTime);
                    velocity *= toMult;

                }


                //Camera logic
                const Camera* getCamera() const {
                    return &camera_;
                }

                void updateCamera(double xPos, double yPos) {

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
                //bool firstMouse = true;

    };

}
