
#include <Entity/Player.hpp>


namespace mav {

    Player::Player(glm::vec3 const& playerStartPosition, float voxelSize)
        : Entity( AABB(playerStartPosition, glm::vec3(voxelSize/2.0f * 0.95f, voxelSize * 0.95f, voxelSize/2.0f * 0.95f)) ),
        camera_(playerStartPosition), voxelSize_(voxelSize)
    {
        front_ = &camera_.Front;
        right_ = &camera_.Right;
        // box.updatePos(position.x, position.y - voxelSize/2.0f, position.z);
    }

    void Player::update(float elapsedTime) {

        //TODO: recup un bool de mouvement ici            
        Entity::update(elapsedTime);

        // Update camera position
        if (true) {
            //TODO: un truc plus clean ?
            camera_.Position = boundingBox_.center;
            camera_.Position.y += voxelSize_ * 0.95 / 2.0f;
            camera_.updateFrustum();
        }

    }

    bool Player::update(float elapsedTime, World const& world) {
        
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

    float Player::calculateNewFOV(float elapsedTime) {

        static const float maximumDiff = 120.0f;

        float timeMaximumDiff = maximumDiff * elapsedTime;

        glm::vec3 specialVelocity(velocity.x, 0.0f, velocity.z);

        float interpolationAlpha = std::min(1.0f, glm::length(specialVelocity) / 15.0f);
        float newFov = baseFov + 60.0f * interpolationAlpha;

        if (newFov == lastFov) return lastFov;
        else if (newFov < lastFov) {
            float diff = lastFov - newFov;
            if (diff < timeMaximumDiff) return newFov;
            else return lastFov - timeMaximumDiff;
        }
        else {
            float diff = newFov - lastFov;
            if (diff < timeMaximumDiff) return newFov;
            else return lastFov + timeMaximumDiff;
        }

    }

    //Camera logic
    Camera* Player::getCamera() {
        return &camera_;
    }

    void Player::updateCamera(double xPos, double yPos) {

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
                
}
