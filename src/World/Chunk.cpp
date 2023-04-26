
#include <World/Chunk.hpp>
#include <Core/Global.hpp>

#include <iostream>

#ifdef TIME
	//Help/debug
	#include <Helper/Benchmark/Profiler.hpp>
#endif

namespace {

	uint8_t vertexAO(bool side1, bool corner, bool side2) {
		if(side1 && side2) {
			return 0;
		}
  		return 3 - (side1 + side2 + corner);
	}

}


namespace mav{

	//TODO: Move this to SimpleVoxel ?
	//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
	//TODO: remplacer ça par un array de glm::ivec3
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

	
	Chunk::Chunk(World* world, int posX, int posY, int posZ, size_t size, float voxelSize, const VoxelMapGenerator * voxelMapGenerator)
		: Drawable(true, 10, {{3}, {3}, {2}, {1}, {1}}, world->shader), state(0), posX_(posX), posY_(posY), posZ_(posZ),
		size_((int)size), voxelSize_(voxelSize), positionOffsets_( -((float)(size_ - 1) / 2.0f) * voxelSize ),
		centerWorldPosition_(posX*(size_*voxelSize), posY*(size_*voxelSize), posZ*(size_*voxelSize)),
		collisionBox_(glm::vec3(centerWorldPosition_), size_*voxelSize),
		world_(world), voxelMapGenerator_(voxelMapGenerator)
#ifndef NDEBUG
		, chunkSides(&Global::debugShader, world->environment, {
			{0.1f, 0.1f, 0.1f},
			{0.5f, 0.5f, 0.5f},
			{1.0f, 1.0f, 1.0f}
		}, size_*voxelSize, centerWorldPosition_ )
#endif
		{

			#ifndef NDEBUG
				chunkSides.initialize(true);
    		#endif

		}


	void Chunk::generateVoxels() {
		
		#ifdef TIME
			Profiler profiler("Generate voxels");
		#endif

		// Move voxelMap and prepare indices size
		VoxelData voxelData = voxelMapGenerator_->generate(posX_, posY_, posZ_);
		
		VoxelMap const& voxelMap = voxelData.map;
		voxels_.data.reserve(voxelData.count);
		
		voxels_.initializeIndices(size_);

		// Calculate the coordinates of each points
		for (size_t x = 0; x < size_; ++x) {
			for (size_t y = 0; y < size_; ++y) {
				for (size_t z = 0; z < size_; ++z) {
					
					if( voxelMap[x][y][z] == 0 ) continue;

					// Position in the chunk + Offset to center the chunk + position in the world
					float xPos = (x * voxelSize_) + positionOffsets_ + (posX_ * size_ * voxelSize_);
					float yPos = (y * voxelSize_) + positionOffsets_ + (posY_ * size_ * voxelSize_);
					float zPos = (z * voxelSize_) + positionOffsets_ + (posZ_ * size_ * voxelSize_);

					voxels_.voxelIndices[x][y][z] = voxels_.data.size();
					voxels_.data.emplace_back(glm::vec3(xPos, yPos, zPos), glm::ivec3(x, y, z), voxelMap[x][y][z]);

					//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
					for (uint8_t faceIndex = 0; faceIndex < Chunk::faceToNeighborOffset.size(); ++faceIndex) {

						//TODO: On a observé un gain de temps au passage de glm::vec3 à utiliser des entiers direct. Chercher des endroits où on peut optimiser pareil
						int foundVoxel = findVoxel(
								x + (Chunk::faceToNeighborOffset[faceIndex].first == 0) * Chunk::faceToNeighborOffset[faceIndex].second,
								y + (Chunk::faceToNeighborOffset[faceIndex].first == 1) * Chunk::faceToNeighborOffset[faceIndex].second,
								z + (Chunk::faceToNeighborOffset[faceIndex].first == 2) * Chunk::faceToNeighborOffset[faceIndex].second,
								voxelMap
						);
						// If the positions are out of bound, test with the voxelTester function
						
						//TODO: Rendre ça paramétrable
						if (true) {
							if (foundVoxel == 2) {
								foundVoxel = voxelMapGenerator_->isIn(
									xPos + voxelSize_ * (Chunk::faceToNeighborOffset[faceIndex].first == 0) * Chunk::faceToNeighborOffset[faceIndex].second,
									yPos + voxelSize_ * (Chunk::faceToNeighborOffset[faceIndex].first == 1) * Chunk::faceToNeighborOffset[faceIndex].second,
									zPos + voxelSize_ * (Chunk::faceToNeighborOffset[faceIndex].first == 2) * Chunk::faceToNeighborOffset[faceIndex].second
								);
							}
						}

						if(foundVoxel == 1) {
							voxels_.data.back().setFaceState(faceIndex, false);							
						}

					}

				}
			}

		}

	}

	int Chunk::findVoxel(glm::vec3 const& position, VoxelMap const& voxelMap) const {

		if (position.x < 0 || position.y < 0 || position.z < 0 || position.x >= size_ || position.y >= size_ || position.z >= size_) return 2;

		return voxelMap[position.x][position.y][position.z] != 0;
	}

	int Chunk::findVoxel(int x, int y, int z, VoxelMap const& voxelMap) const {

		if (x < 0 || y < 0 || z < 0 || x >= size_ || y >= size_ || z >= size_) return 2;

		return voxelMap[x][y][z] != 0;
	}

	int Chunk::findVoxel(glm::ivec3 const& position) const {

		if (position.x < 0 || position.y < 0 || position.z < 0 || position.x >= size_ || position.y >= size_ || position.z >= size_) return 2;

		return voxels_.voxelIndices[position.x][position.y][position.z] != -1;
	}

	int Chunk::findVoxel(int x, int y, int z) const {

		if (x < 0 || y < 0 || z < 0 || x >= size_ || y >= size_ || z >= size_) return 2;

		return voxels_.voxelIndices[x][y][z] != -1;
	}

	SimpleVoxel* Chunk::unsafeGetVoxel(int x, int y, int z) {

		// if (x < 0 || y < 0 || z < 0 || x >= size_ || y >= size_ || z >= size_) {
		//  	return nullptr;
		// }

		int index = voxels_.voxelIndices[x][y][z];
		if(index != -1) return &voxels_.data[ index ];

		return nullptr;

	}

	int Chunk::findAndGetVoxelIndex(glm::ivec3 const& position) {
		if (position.x < 0 || position.y < 0 || position.z < 0
			|| position.x >= size_ || position.y >= size_ || position.z >= size_) return -2;

		return voxels_.voxelIndices[position.x][position.y][position.z];
	}

	std::pair<SimpleVoxel*, Chunk*> Chunk::getSideVoxel(glm::ivec3 voxelPosition, uint8_t side) {

		#ifdef TIME
			Profiler profiler("Get side voxel");
		#endif

		glm::ivec3 neighborPosition = {
				voxelPosition.x + (Chunk::faceToNeighborOffset[side].first == 0) * Chunk::faceToNeighborOffset[side].second,
				voxelPosition.y + (Chunk::faceToNeighborOffset[side].first == 1) * Chunk::faceToNeighborOffset[side].second,
				voxelPosition.z + (Chunk::faceToNeighborOffset[side].first == 2) * Chunk::faceToNeighborOffset[side].second
		};

		int sideVoxelIndex = findAndGetVoxelIndex(neighborPosition);
		if (sideVoxelIndex >= 0) {
			return {&voxels_.data[sideVoxelIndex], nullptr};
		}
		else if (sideVoxelIndex == -2) {

			Chunk* sideChunk = world_->getChunk(
				posX_ + (Chunk::faceToNeighborOffset[side].first == 0) * Chunk::faceToNeighborOffset[side].second,
				posY_ + (Chunk::faceToNeighborOffset[side].first == 1) * Chunk::faceToNeighborOffset[side].second,
				posZ_ + (Chunk::faceToNeighborOffset[side].first == 2) * Chunk::faceToNeighborOffset[side].second
			);

			if (sideChunk == nullptr) {
				//Special case, the position is not in the current chunk but the side voxel does not exist
				return {nullptr, nullptr};
			}

			neighborPosition[Chunk::faceToNeighborOffset[side].first] -= size_ * Chunk::faceToNeighborOffset[side].second;
			int sideChunkVoxelIndex = sideChunk->findAndGetVoxelIndex(neighborPosition);

			if (sideChunkVoxelIndex >= 0) {
				return {&sideChunk->voxels_.data[sideChunkVoxelIndex], sideChunk};
			}
			else {
				return {nullptr, sideChunk};
			}

		}

		//Here, the position is in the current chunk but the voxel is empty
		return {nullptr, nullptr};
	}

	bool Chunk::deleteVoxel(glm::ivec3 position) {

		int voxelIndex = voxels_.voxelIndices[position.x][position.y][position.z];
		if (voxelIndex == -1) return false;

		#ifdef TIME
			Profiler profiler("Delete voxel");
		#endif

		voxels_.voxelIndices[position.x][position.y][position.z] = -1;

		// Move the last voxel at the place of the old voxel to prevent moving all the indexes
		// and put the correct index in the indexes map for the moved voxel
		// (only if it was not already the last voxel)
		if (voxelIndex != voxels_.data.size() - 1) {
			voxels_.data[voxelIndex] = std::move(voxels_.data.back());

			glm::ivec3 const& movedChunkPosition = voxels_.data[voxelIndex].getChunkPosition();
			voxels_.voxelIndices[movedChunkPosition.x][movedChunkPosition.y][movedChunkPosition.z] = voxelIndex;
		}
		
		voxels_.data.pop_back();

		//And correct neighbors face state
		for (uint8_t faceIndex = 0; faceIndex < Chunk::faceToNeighborOffset.size(); ++faceIndex) {

			auto [sideVoxel, sideChunk] = getSideVoxel(position, faceIndex);
			if (sideVoxel) {
				sideVoxel->setFaceState(faceToInverseFace[faceIndex], true);
			}
			//We force to regenerate even if it was not edited to regenrate AO
			if (sideChunk) world_->needToRegenerateChunks.insert(sideChunk);

		}

		//We notify that we need to regenerate this chunk
		world_->needToRegenerateChunks.insert(this);

		return true;
	}

	Chunk* Chunk::getChunk(glm::ivec3& voxelChunkPosition) const {
		
		glm::ivec3 positionOffset(0);

		//If an axe is negative, remove 1 from its coordinate
		if (voxelChunkPosition.x < 0) --positionOffset.x;
		if (voxelChunkPosition.y < 0) --positionOffset.y;
		if (voxelChunkPosition.z < 0) --positionOffset.z;

		//Add the offset of each axe
		positionOffset.x += voxelChunkPosition.x/size_;
		positionOffset.y += voxelChunkPosition.y/size_;
		positionOffset.z += voxelChunkPosition.z/size_;

		Chunk* foundChunk = world_->getChunk(posX_ + positionOffset.x, posY_ + positionOffset.y, posZ_ + positionOffset.z);
		
		if (foundChunk) {

			//Replace the voxel chunk position in the found chunk space
			voxelChunkPosition.x -= positionOffset.x * size_;
			voxelChunkPosition.y -= positionOffset.y * size_;
			voxelChunkPosition.z -= positionOffset.z * size_;

		}

		return foundChunk;

	}

	void Chunk::addVoxel(glm::ivec3 position, int newVoxelId) {

		Chunk* affectedChunk = this;

		//In this case, we need to insert it in the neibor chunk
		if (position.x < 0 || position.y < 0 || position.z < 0 || position.x >= size_ || position.y >= size_ || position.z >= size_) {
			affectedChunk = getChunk(position);
		}

		if (affectedChunk) affectedChunk->unsafeAddVoxel(position, newVoxelId);

	}

	void Chunk::unsafeAddVoxel(glm::ivec3 position, int newVoxelId) {

		size_t x = position.x;
		size_t y = position.y;
		size_t z = position.z;

		// Position in the chunk + Offset to center the chunk + position in the world
		float xPos = (x * voxelSize_) + positionOffsets_ + (posX_ * size_ * voxelSize_);
		float yPos = (y * voxelSize_) + positionOffsets_ + (posY_ * size_ * voxelSize_);
		float zPos = (z * voxelSize_) + positionOffsets_ + (posZ_ * size_ * voxelSize_);

		voxels_.voxelIndices[x][y][z] = voxels_.data.size();
		voxels_.data.emplace_back(glm::vec3(xPos, yPos, zPos), position, newVoxelId);

		//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
		for (uint8_t faceIndex = 0; faceIndex < Chunk::faceToNeighborOffset.size(); ++faceIndex) {

			auto [sideVoxel, sideChunk] = getSideVoxel(position, faceIndex);
			if (sideVoxel) {
				voxels_.data.back().setFaceState(faceIndex, false);
				sideVoxel->setFaceState(Chunk::faceToInverseFace[faceIndex], false);
			}

			// If the voxel is not from the current chunk, notify that we need to regenrate for AO
			if (sideChunk) world_->needToRegenerateChunks.insert(sideChunk);
		}

		//Notify that we need to regenerate this chunk
		world_->needToRegenerateChunks.insert(this);

	}

	std::array<float, 4> Chunk::generateAmbientOcclusion(SimpleVoxel const& voxel, uint8_t faceIndex) const {
		
		#ifdef TIME
			Profiler profiler("AO");
		#endif

		static const std::array<float, 4> AOcurve { 0.25f, 0.6f, 0.8f, 1.0f };

		//Faces order
		//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
		static const std::vector<std::vector<glm::ivec3>> positionToLookUp {
			//bottom
			{ {-1, -1, 0}, {-1, -1, 1}, {0, -1, 1}, {1, -1, 1}, {1, -1, 0}, {1, -1, -1}, {0, -1, -1}, {-1, -1, -1} },
			//front
			{ {-1, 0, 1}, {-1, 1, 1}, {0, 1, 1}, {1, 1, 1}, {1, 0, 1}, {1, -1, 1}, {0, -1, 1}, {-1, -1, 1} },
			//right
			{ {1, 0, 1}, {1, 1, 1}, {1, 1, 0}, {1, 1, -1}, {1, 0, -1}, {1, -1, -1}, {1, -1, 0}, {1, -1, 1} },
			//back
			{ {1, 0, -1}, {1, 1, -1}, {0, 1, -1}, {-1, 1, -1}, {-1, 0, -1}, {-1, -1, -1}, {0, -1, -1}, {1, -1, -1} },
			//left
			{ {-1, 0, -1}, {-1, 1, -1}, {-1, 1, 0}, {-1, 1, 1}, {-1, 0, 1}, {-1, -1, 1}, {-1, -1, 0}, {-1, -1, -1} },
			//top
			{ {-1, 1, 0}, {-1, 1, -1}, {0, 1, -1}, {1, 1, -1}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1}, {-1, 1, 1} }
		};

		glm::ivec3 const& voxelChunkPosition = voxel.getChunkPosition();
		glm::vec3 const& voxelWorldPosition = voxel.getPosition();

		std::vector<bool> foundVoxels;
		foundVoxels.resize(8);
		for(uint8_t i = 0; i < 8; ++i) {
			
			glm::ivec3 neibhborChunkPosition = voxelChunkPosition + positionToLookUp[faceIndex][i];
			int foundVoxel = findVoxel(neibhborChunkPosition);
			if (foundVoxel == 2) {

				const Chunk * neihborChunk = getChunk(neibhborChunkPosition);
				if (neihborChunk) {
					foundVoxel = neihborChunk->findVoxel(neibhborChunkPosition);
				}
				else {
					foundVoxel = voxelMapGenerator_->isIn(
						voxelWorldPosition.x + voxelSize_ * positionToLookUp[faceIndex][i].x,
						voxelWorldPosition.y + voxelSize_ * positionToLookUp[faceIndex][i].y,
						voxelWorldPosition.z + voxelSize_ * positionToLookUp[faceIndex][i].z
					);
				}
			}
			
			if (foundVoxel == 1) foundVoxels[i] = true;
			else foundVoxels[i] = false;
			
		}

		//Vertices order
		//top left, bottom left, bottom right, top right
		return {
			AOcurve[vertexAO(foundVoxels[0], foundVoxels[1], foundVoxels[2])],
			AOcurve[vertexAO(foundVoxels[6], foundVoxels[7], foundVoxels[0])],
			AOcurve[vertexAO(foundVoxels[4], foundVoxels[5], foundVoxels[6])],
			AOcurve[vertexAO(foundVoxels[2], foundVoxels[3], foundVoxels[4])]
		};

	}

	void Chunk::generateVertices() {

		#ifdef TIME
			Profiler profiler("Generate vertices");
		#endif

		// We first make sure that the vectors are empty
		vertices_.clear();
		indices_.clear();
		
		size_t insertedFaces = 0;
		for (size_t i = 0; i < voxels_.data.size(); ++i) {
			SimpleVoxel const& voxel = voxels_.data[i];

			for (uint8_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
				
				// Here the face is not shown
				if (!voxel.getFaceState(faceIndex)) continue;

				//Ambient occlusion
				//TODO: benchmark le temps que prends cette nouvelle fonction en tout
				std::array<float, 4> AO = generateAmbientOcclusion(voxel, faceIndex);

				// glm::vec3 voxelPosition = voxel.getPosition();
				// std::array<float, 4> AO = generateAmbientOcclusion(glm::ivec3(
				// 	(voxelPosition.x - positionOffsets_ - (posX_ * size_ * voxelSize_)) / voxelSize_,
				// 	(voxelPosition.y - positionOffsets_ - (posY_ * size_ * voxelSize_)) / voxelSize_,
				// 	(voxelPosition.z - positionOffsets_ - (posZ_ * size_ * voxelSize_)) / voxelSize_), faceIndex
				// );

				std::vector<float> face = voxel.getFace(faceIndex, AO);

				//We insert the face vertices...
				vertices_.insert(vertices_.end(), std::make_move_iterator(face.begin()), std::make_move_iterator(face.end()));

				//...And it's corresponding indices for the EBO
				size_t currentIndexOffset = (insertedFaces++ * 4);

				//We flip the vertices depending on the strongest diagonal to lower the effect of quad interpolation
				if (AO[0] + AO[2] > AO[1] + AO[3]) {
					for (uint8_t index : SimpleVoxel::flippedVerticesIndices) {
						indices_.push_back(currentIndexOffset + index);
					}
				}
				else {
					for (uint8_t index : SimpleVoxel::verticesIndices) {
						indices_.push_back(currentIndexOffset + index);
					}
				}

			}

		}

	}

	void Chunk::draw(){

		if (indicesSize_ == 0) return;

		//Verify if chunks is inside camera frustum
		bool isVisible = world_->environment->camera->frustum.collide(collisionBox_);

		#ifndef NDEBUG
			//We always want to draw the debug chunk sides so we do it before the checks
			if (isVisible) chunkSides.setColor(glm::vec3(1.0f));
			else chunkSides.setColor(glm::vec3(1.0f, 0.0f, 0.0f));
			
			chunkSides.draw();
		#endif

		if (!isVisible) return;

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

		world_->shader->setFloat("time", world_->environment->totalElapsedTime);

		//Calcule camera

		glm::mat4 view(world_->environment->camera->GetViewMatrix());
		//glm::mat4 view(glm::lookAt(cameraPtr_->Position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

		world_->shader->setMat4("view", view);

		//Position de la cam
		world_->shader->setVec3("viewPos", world_->environment->camera->Position);

		world_->shader->setMat4("projection", world_->environment->camera->Projection);


		glDrawElements(GL_TRIANGLES, (int)indicesSize_, GL_UNSIGNED_INT, 0);

	}

}