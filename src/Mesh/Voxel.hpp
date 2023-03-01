#pragma once

#include <Mesh/Mesh.hpp>

#include <GLObject/Shader.hpp>
#include <Environment/Environment.hpp>
#include <GLObject/GLObject.hpp>

#include <glm/glm.hpp>

#include <vector>


namespace mav {

    class Voxel : public Mesh {

		public:

			Voxel(Shader* shaderPtr, Environment* environment, Material material, float size = 1, glm::vec3 position = glm::vec3(0.0f));

			void init();

			void generateVertices();

			void updateVAO();

    };

}