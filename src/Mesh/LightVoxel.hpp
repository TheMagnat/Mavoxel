#pragma once

#include <Mesh/Voxel.hpp>

#include <Environment/Environment.hpp>

#include <glm/glm.hpp>

#include <vector>


namespace mav {

    class LightVoxel : public Voxel {

		public:

			LightVoxel(Environment* environment, Material material, size_t size = 10);

      void updateShader(vuw::Shader* shader, uint32_t currentFrame) const override;

    };

}