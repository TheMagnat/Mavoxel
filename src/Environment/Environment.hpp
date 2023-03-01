#pragma once

#include <Mesh/Mesh.hpp>
#include <GLObject/Camera.hpp>

namespace mav {

    class Mesh;

    struct Environment {

        Mesh* sun;
        Camera* camera;

    };

}
