
#pragma once

#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>


namespace mav {

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
        alignas(16) glm::mat4 projection;
        alignas(16) glm::mat4 view;
        alignas(16) SunObject sun;
        alignas(16) float time;
        alignas(16) glm::vec3 voxelCursorPosition;
        alignas(16) glm::vec3 faceCursorNormal;
        alignas(16) glm::vec2 jitter;
    };

    struct WorldOctreeInformations {
        glm::ivec3 centerChunkPosition;
        int depth;
        int len;
        float voxelSize;
    };

    struct TestInformations {
        glm::vec2 sunScreenPos;
    };

    struct FilterInformations {
        alignas(16) glm::mat4 oldProjectionView;
        alignas(16) glm::mat4 newProjectionView;
        alignas(16) glm::mat4 view;
        alignas(16) int debug;
    };

    

    struct Material {
        alignas(16) glm::vec3 ambient;
        alignas(16) glm::vec3 diffuse;
        alignas(16) glm::vec3 specular;

        float shininess;
    };

}
