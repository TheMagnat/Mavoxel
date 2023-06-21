
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
