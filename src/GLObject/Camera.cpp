
#include "Camera.hpp"

#include <Core/Global.hpp>

namespace mav {

Camera::Camera(glm::vec3 const& position, glm::vec3 const& up, float yaw, float pitch)
	: Position(position), Front(0.0f, 0.0f, -1.0f), WorldUp(up), Yaw(yaw), Pitch(pitch), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM),
      maintainFrustum(true)
{
    updateCameraVectors();

    //Default perspective
    setPerspectiveProjectionMatrix(glm::radians(45.0f), (float)mav::Global::width / (float)mav::Global::height, 0.1f, 200.0f);
    updateFrustum();
}


Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) 
: Position(posX, posY, posZ), Front(0.0f, 0.0f, -1.0f), WorldUp(upX, upY, upZ), Yaw(yaw), Pitch(pitch), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM),
  maintainFrustum(true)
{
    updateCameraVectors();

    //Default perspective
    setPerspectiveProjectionMatrix(glm::radians(45.0f), (float)mav::Global::width / (float)mav::Global::height, 0.1f, 200.0f);
    updateFrustum();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::setPerspectiveProjectionMatrix(float fovY, float aspect, float zNear, float zFar) {
    Projection = glm::perspective(fovY, aspect, zNear, zFar);
    if (maintainFrustum) frustum.updatePerspective(fovY, aspect, zNear, zFar);
}

void Camera::updateFrustum() {
    frustum.updateCamera(this);
}

void Camera::ProcessKeyboard(glm::vec3 velocity, float deltaTime){
    Position += velocity * deltaTime;

    // Keep frustum up to date
    if (maintainFrustum) updateFrustum();
}

void Camera::ProcessKeyboard(glm::vec3 velocity){
    Position += velocity;

    // Keep frustum up to date
    if (maintainFrustum) updateFrustum();
}

void Camera::ProcessKeyboard(Camera_Movement direction, float velocity, float deltaTime){
    float velocityByTime = velocity * deltaTime;
    if (direction == FORWARD)
    	Position += Front * velocityByTime;
        //Position += glm::vec3(Front.x * velocityByTime, Position.y, Front.z * velocityByTime);
    if (direction == BACKWARD)
    	Position -= Front * velocityByTime;
        //Position -= glm::vec3(Front.x * velocityByTime, Position.y, Front.z * velocityByTime);
    if (direction == LEFT)
        Position -= Right * velocityByTime;
    if (direction == RIGHT)
        Position += Right * velocityByTime;

    // Keep frustum up to date
    if (maintainFrustum) updateFrustum();
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch){
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();

    // Keep frustum up to date
    if (maintainFrustum) updateFrustum();
}

void Camera::ProcessMouseScroll(float yoffset){
    if (Zoom >= 1.0f && Zoom <= 45.0f)
        Zoom -= yoffset;
    if (Zoom <= 1.0f)
        Zoom = 1.0f;
    if (Zoom >= 45.0f)
        Zoom = 45.0f;
}

void Camera::updateCameraVectors(){
    // Calculate the new Front vector
    glm::vec3 front(cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)), sin(glm::radians(Pitch)), sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)));
    Front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up    = glm::normalize(glm::cross(Right, Front));
}


}