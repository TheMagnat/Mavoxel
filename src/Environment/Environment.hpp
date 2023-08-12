#pragma once

#include <Mesh/Mesh.hpp>
#include <GraphicObjects/Camera.hpp>

#include <Collision/CollisionInformations.hpp>

namespace mav {

    class Mesh;

    struct Environment {

        const Mesh* sun;
        const Camera* camera;
        float totalElapsedTime = 0.0f;

        const CollisionInformations* collisionInformations = nullptr;

    };

}
