#pragma once

#include "../GLObject/GLObject.hpp"

#include "../GLObject/Shader.hpp"
#include "../GLObject/Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <vector>


namespace mav {

	class Plane {
		
		public:

			Plane(Shader* shaderPtr, Camera* cameraPtr, size_t size = 1);

			void init();

			/*
				Set a flat plane (which will have 2 triangle.)
			*/
			// void set();

			/**
			 *	Set a plane which containe a number verticeNb of vertices,
			 *	and each vertices got it's y position set in picked in the height vector.
			 */
			void set(size_t verticesNb, size_t len, std::vector<float> const& height);

			void setPosition(float x, float y);

			void setRotationMatrice(glm::mat4 const& rotaMat);

			void setSizeVec(glm::vec3 const& sizeVec);

			//NORMALS
			void calculateNormals();
			glm::vec3 calculateNormal(int x1, int z1, int x2, int z2, int x3, int z3);
			glm::vec3 calculateNormal(int face);

			/*
				Update the openGL object when an edition is done to the data.
			*/
			void update();

			void draw();

		protected:

			//Size and position of the Plane in the world
			size_t size_;
			float x_;
			float y_;


			glm::vec3 sizeVec_;

			glm::mat4 rotationMat_;


			//Vertices info
			size_t row_;
			size_t len_;


			//GL
			Shader* shaderPtr_;
			Camera* cameraPtr_;

			VAO vao_;
			std::vector<float> vertices_;

			size_t indicesNb_;
			std::vector<int> indices_;



	};


	class ColorPlane : public Plane {
		
		public:

			//ColorPlane(Shader* shaderPtr, Camera* cameraPtr, size_t size = 1);

			void set(size_t verticesNb, size_t len, std::vector<float> const& height, std::vector<glm::vec3> const& colors);

			void update();

	};

}