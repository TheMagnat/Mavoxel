
#pragma once

#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>


struct ModelViewProjectionObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 modelNormal;
};

struct ModelViewProjectionObjectNoNormal {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct ViewProjectionObject {
    glm::mat4 view;
    glm::mat4 projection;
};

struct LightObject {
    alignas(16) glm::vec3 position;

    alignas(16) glm::vec3 ambient;
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
};

struct SunObject {
    alignas(16) glm::vec3 position;
};

//TODO: voir si il faut faire alignas(16)
struct CameraObject {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 front;
    alignas(16) glm::vec3 up;
    alignas(16) glm::vec3 right;
};

struct RayCastInformations {
    alignas(16) float xRatio;
    alignas(16) CameraObject camera;
    alignas(16) SunObject sun;
};
