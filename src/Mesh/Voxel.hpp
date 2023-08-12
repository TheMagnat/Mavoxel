#pragma once

#include <Mesh/Mesh.hpp>

#include <Environment/Environment.hpp>

#include <glm/glm.hpp>

#include <vector>


namespace mav {

    class Voxel : public Mesh {

		public:

			Voxel(Environment* environment, Material material, float size = 1, glm::vec3 position = glm::vec3(0.0f));

			void generateVertices() override;

			std::vector<uint32_t> getVertexAttributesSizes() const override;
            virtual void updateShader(vuw::Shader* shader, uint32_t currentFrame) const override;

    };

}