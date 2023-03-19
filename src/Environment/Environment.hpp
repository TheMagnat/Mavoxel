#pragma once

#include <Mesh/Mesh.hpp>
#include <GLObject/Camera.hpp>

namespace mav {

    class Mesh;

    struct Environment {

        const Mesh* sun;
        const Camera* camera;

    };

}
