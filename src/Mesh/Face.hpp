#pragma once

#include <Mesh/Mesh.hpp>

#include <VulkanWrapper/Shader.hpp>
#include <Environment/Environment.hpp>

#include <glm/glm.hpp>

#include <vector>
#include <array>


namespace mav {

    class Face : public Mesh {

		public:

			Face(Environment* environment, Material material, float size = 1, glm::vec3 position = glm::vec3(0.0f));

			void generateVertices() override;
			void generateVertices(std::array<glm::vec3, 4> const& points);

			std::vector<uint32_t> getVertexAttributesSizes() const override;
            void updateShader(vuw::Shader* shader, uint32_t currentFrame) const override;

    };

}