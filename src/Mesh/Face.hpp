#pragma once

#include <Mesh/Mesh.hpp>

#include <GLObject/Shader.hpp>
#include <Environment/Environment.hpp>
#include <GLObject/GLObject.hpp>

#include <glm/glm.hpp>

#include <vector>
#include <array>


namespace mav {

    class Face : public Mesh {

		public:

			Face(Shader* shaderPtr, Environment* environment, Material material, float size = 1, glm::vec3 position = glm::vec3(0.0f));

			void generateVertices() override;
			void generateVertices(std::array<glm::vec3, 4> const& points);

			void draw();

    };

}