
#include <World/Chunk.hpp>
#include <Core/Global.hpp>

#include <iostream>

namespace mav{
	
	Chunk::Chunk(World* world, int posX, int posZ, size_t size, size_t voxelSize)
		: posX_(posX), posZ_(posZ), size_(size), voxelSize_(voxelSize), world_(world) {

		vao_.init(true);

	}

	//TODO: User a generator function (that take an X, Y and Z function)
	void Chunk::generateVoxels(){

		Material grassMaterial {
			{0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.5f, 0.5f, 0.5f},
			32.0f
		};

		for (size_t x = 0; x < size_; ++x)
		{
			for (size_t z = 0; z < size_; ++z)
			{

				float xPos = x*voxelSize_;
				float zPos = z*voxelSize_;
				float yPos = (x+z)*voxelSize_;

				size_t newIndex = voxels_.size();

				//TODO: Also fill the faces state with setFaceState
				voxels_.emplace_back(glm::vec3(xPos, yPos, zPos));
				
				voxelCoordToIndex_[x][0][z] = newIndex;

			}
			
		}

	}

	void Chunk::generateVertices() {
	
		for (size_t i = 0; i < voxels_.size(); ++i) {
			SimpleVoxel const& voxel = voxels_[i];
			std::array<std::vector<float>, 6> const& faces = voxel.getFaces();
			
			for (size_t j = 0; j < faces.size(); ++j) {
				std::vector<float> const& face = faces[j];

				//We insert the face vertices...
				//TODO: Maybe use the information on the face to see if we need to insert it or not			
				vertices_.insert(vertices_.end(), face.begin(), face.end());

				//...And it's corresponding indices for the EBO
				size_t currentIndexOffset = (i * 6 * 4) + (j * 4);
				for (uint8_t indice : SimpleVoxel::verticesIndices) {
					indices_.push_back(currentIndexOffset + indice);
				}

			}

		}

	}

	void Chunk::graphicUpdate(){

        std::vector<VAO::Attribute> allAttribute = {{3}, {3}, {1}};

		vao_.setAll(vertices_, 7, allAttribute, indices_);
    }

	void Chunk::draw(){

		glBindVertexArray(vao_.get());
		

		//SET LE SHADER
		world_->shader->use();
    
		glm::mat4 model = glm::mat4(1.0f);
        //TODO: ADD CHUNK TRANSLATION
		//model = model * translationMatrix_;
        
		//model = glm::rotate(model, 45.0f, glm::vec3(0.0f, 1.0f, 1.0f));
		//model = model * rotationMat_;
		//TODO: Size ?
		//model = glm::scale(model, glm::vec3(100));


		world_->shader->setMat4("model", model);
		world_->shader->setMat3("modelNormal", glm::mat3(glm::transpose(glm::inverse(model))));

		////TOUT LES NEED
		world_->shader->use();

		/*
		world_->shader->setVec3("material.ambient", material.ambient);
		world_->shader->setVec3("material.diffuse", material.diffuse);
		world_->shader->setVec3("material.specular", material.specular);
		world_->shader->setFloat("material.shininess", material.shininess);
		*/

		world_->shader->setVec3("light.ambient",  world_->environment->sun->material.ambient);
		world_->shader->setVec3("light.diffuse",  world_->environment->sun->material.diffuse); // assombri un peu la lumière pour correspondre à la scène
		world_->shader->setVec3("light.specular", world_->environment->sun->material.specular);

		world_->shader->setVec3("light.position", world_->environment->sun->getPosition());

		// world_->shader->setFloat("light.constant",  1.0f);
		// world_->shader->setFloat("light.linear",    0.09f);
		// world_->shader->setFloat("light.quadratic", 0.032f);


		//world_->shader->setVec3("light.position", glm::vec3(0, 50, 100));



		//Calcule camera

		glm::mat4 view(world_->environment->camera->GetViewMatrix());
		//glm::mat4 view(glm::lookAt(cameraPtr_->Position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

		world_->shader->setMat4("view", view);

		//Position de la cam
		world_->shader->setVec3("viewPos", world_->environment->camera->Position);

		world_->shader->setMat4("projection", glm::perspective(glm::radians(45.0f), (float)mav::Global::width / (float)mav::Global::height, 0.1f, 2000.0f));


		glDrawElements(GL_TRIANGLES, (int)indices_.size(), GL_UNSIGNED_INT, 0);


	}

}