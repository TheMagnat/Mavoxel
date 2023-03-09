
#include <World/Chunk.hpp>
#include <Core/Global.hpp>

#include <iostream>

namespace mav{

	//TODO: Move this to SimpleVoxel ?
	//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
	std::array<std::pair<int, int>, 6> Chunk::faceToNeighborOffset = {{
		{1, -1},
		{2, 1},
		{0, 1},
		{2, -1},
		{0, -1},
		{1, 1}
	}};

	//TODO: Move this to SimpleVoxel.
	//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
	std::array<uint8_t, 6> Chunk::faceToInverseFace = {
		{5, 3, 4, 1, 2, 0}
	};

	
	Chunk::Chunk(World* world, int posX, int posY, int posZ, int size, float voxelSize)
		: state(0), posX_(posX), posY_(posY), posZ_(posZ), size_(size), voxelSize_(voxelSize), world_(world) {

		vao_.init(true);

	}

	void Chunk::generateVoxels(VoxelGeneratorFunc generator) {

		voxels_.initializeIndices(size_);
		voxels_.initializeMap(size_);

		float positionOffsets = - ((size_) / 2.0f) * voxelSize_;

		// Calculate the coordinates of each points
		for (size_t x = 0; x < size_; ++x) {
			for (size_t y = 0; y < size_; ++y) {
				for (size_t z = 0; z < size_; ++z) {

					// Position in the chunk + Offset to center the chunk + position in the world
					float xPos = (x * voxelSize_) + positionOffsets + (posX_ * size_ * voxelSize_);
					float yPos = (y * voxelSize_) + positionOffsets + (posY_ * size_ * voxelSize_);
					float zPos = (z * voxelSize_) + positionOffsets + (posZ_ * size_ * voxelSize_);

					int genValue = generator(xPos, yPos, zPos);
					if(genValue == 0)
						continue;

					voxels_.voxelMap[x][y][z] = genValue;
					voxels_.voxelIndices[x][y][z] = voxels_.data.size();
					voxels_.data.emplace_back(glm::vec3(xPos, yPos, zPos), 0, voxelSize_);

					//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
					for (uint8_t faceIndex = 0; faceIndex < Chunk::faceToNeighborOffset.size(); ++faceIndex) {

						glm::vec3 positionToCheck = glm::vec3(xPos, yPos, zPos);
						positionToCheck[Chunk::faceToNeighborOffset[faceIndex].first] += Chunk::faceToNeighborOffset[faceIndex].second * voxelSize_;

						bool foundVoxel = generator(positionToCheck.x, positionToCheck.y, positionToCheck.z);
						if(foundVoxel) {
							voxels_.data.back().setFaceState(faceIndex, false);							
						}

					}

				}
			}
		}



	}

	//TODO: Aussi recevoir le generator pour les côtés du chunk
	void Chunk::generateVoxels(VoxelMap& voxelsMap) {
		
		// Move voxelMap and prepare indices size
		voxels_.voxelMap = std::move(voxelsMap);
		voxels_.initializeIndices(size_);

		// Calculate the coordinates of each points
		float positionOffsets = - ((size_) / 2.0f) * voxelSize_;
		for (size_t x = 0; x < size_; ++x) {
			for (size_t y = 0; y < size_; ++y) {
				for (size_t z = 0; z < size_; ++z) {
					
					if( voxels_.voxelMap[x][y][z] == 0 ) continue;

					// Position in the chunk + Offset to center the chunk + position in the world
					float xPos = (x * voxelSize_) + positionOffsets + (posX_ * size_ * voxelSize_);
					float yPos = (y * voxelSize_) + positionOffsets + (posY_ * size_ * voxelSize_);
					float zPos = (z * voxelSize_) + positionOffsets + (posZ_ * size_ * voxelSize_);

					voxels_.voxelIndices[x][y][z] = voxels_.data.size();
					voxels_.data.emplace_back(glm::vec3(xPos, yPos, zPos), 0, voxelSize_);

					//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
					for (uint8_t faceIndex = 0; faceIndex < Chunk::faceToNeighborOffset.size(); ++faceIndex) {

						glm::vec3 positionToCheck = glm::vec3(x, y, z);
						positionToCheck[Chunk::faceToNeighborOffset[faceIndex].first] += Chunk::faceToNeighborOffset[faceIndex].second;

						bool foundVoxel = findVoxel(positionToCheck);
						if(foundVoxel) {
							voxels_.data.back().setFaceState(faceIndex, false);							
						}

					}

				}
			}

		}

	}

	bool Chunk::findVoxel(glm::vec3 const& position) {

		if (position.x < 0 || position.y < 0 || position.z < 0 || position.x >= size_ || position.y >= size_ || position.z >= size_) return false;

		return voxels_.voxelMap[position.x][position.y][position.z] != 0;
	}

	void Chunk::generateVertices() {
		
		size_t insertedFaces = 0;
		for (size_t i = 0; i < voxels_.data.size(); ++i) {
			SimpleVoxel const& voxel = voxels_.data[i];

			for (uint8_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
				
				// Here the face is not shown
				if (!voxel.getFaceState(faceIndex)) continue;

				std::vector<float> face = voxel.getFace(faceIndex);

				//We insert the face vertices...
				vertices_.insert(vertices_.end(), std::make_move_iterator(face.begin()), std::make_move_iterator(face.end()));

				//...And it's corresponding indices for the EBO
				size_t currentIndexOffset = (insertedFaces++ * 4);
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
		//model = glm::scale(model, glm::vec3(voxelSize_));


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