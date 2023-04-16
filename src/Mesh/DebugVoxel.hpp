#pragma once

#include <Mesh/Mesh.hpp>

#include <GLObject/Shader.hpp>
#include <Environment/Environment.hpp>
#include <GLObject/GLObject.hpp>

#include <glm/glm.hpp>

#include <vector>


namespace mav {

    class DebugVoxel : public Mesh {

		public:

			DebugVoxel(Shader* shaderPtr, Environment* environment, Material material, float size = 1, glm::vec3 position = glm::vec3(0.0f));

			void generateVertices() override;

			void draw() const override;

			//Unique
			void setColor(glm::vec3 const& color);
		
		private:

			glm::vec3 color_;
    };

}