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

			LightVoxel(Environment* environment, Material material, size_t size = 10);

      void updateUniforms(vuw::Shader* shader, uint32_t currentFrame) const override;

    };

}