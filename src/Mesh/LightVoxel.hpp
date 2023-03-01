#pragma once

#include <Mesh/Voxel.hpp>

#include <GLObject/Shader.hpp>
#include <Environment/Environment.hpp>
#include <GLObject/GLObject.hpp>

#include <glm/glm.hpp>

#include <vector>


namespace mav {

    class LightVoxel : public Voxel {

		public:

			LightVoxel(Shader* shaderPtr, Environment* environment, Material material, size_t size = 10);

    };

}