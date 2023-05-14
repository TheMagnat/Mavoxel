
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