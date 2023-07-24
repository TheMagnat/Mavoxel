
#pragma once

#include <GLObject/Camera.hpp>
#include <Entity/Entity.hpp>

namespace mav {

    class Player : public Entity {

            public:
                Player(glm::vec3 const& playerStartPosition, float voxelSize);

                void update(float elapsedTime);

                bool update(float elapsedTime, World const& world);

                //TODO: retirer ?
                float calculateNewFOV(float elapsedTime);

                //Camera logic
                Camera* getCamera();

                void updateCamera(double xPos, double yPos);

                //TODO: in cpp
                //For debug purpose
                void setState(glm::vec3 position, float yaw, float pitch) {
                    
                    boundingBox_.center = position;
                    camera_.Position = boundingBox_.center;
                    camera_.Position.y += voxelSize_ * 0.95 / 2.0f;

                    camera_.Yaw = yaw;
                    camera_.Pitch = pitch;

                    camera_.updateCameraVectors();
                    camera_.updateFrustum();

                }

            private:
                //Camera logic
                Camera camera_;
                double lastX;
                double lastY;
                bool firstMouse = true;

                float baseFov = 45.0f;
                float lastFov = 45.0f;

                //World data
                float voxelSize_;

    };

}
