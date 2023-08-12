#pragma once

#include <Mesh/Mesh.hpp>

#include <Environment/Environment.hpp>

#include <glm/glm.hpp>

#include <vector>


namespace mav {

    class DebugVoxel : public Mesh {

		public:

			DebugVoxel(Environment* environment, Material material, float size = 1, glm::vec3 position = glm::vec3(0.0f));

			void generateVertices() override;

			//Unique
			void setColor(glm::vec3 const& color);

			std::vector<uint32_t> getVertexAttributesSizes() const override;
            void updateShader(vuw::Shader* shader, uint32_t currentFrame) const override;
		
		private:

			glm::vec3 color_;
    };

}