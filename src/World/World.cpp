
#include "World/World.hpp"

#include <iostream>

namespace mav {

	World::World(Shader* shaderPtrP, Environment* environmentP, size_t chunkSize, size_t voxelSize)
		: chunkSize_(chunkSize), voxelSize_(voxelSize), shaderPtr(shaderPtrP), environment(environmentP) {}

	void World::createChunk(int chunkPosX, int chunkPosZ){

		size_t newIndex = allChunk_.size();

		allChunk_.emplace_back(this, chunkPosX, chunkPosZ, chunkSize_, voxelSize_);
		allChunk_.back().generate();

		chunkCoordToIndex_[chunkPosX][chunkPosZ] = newIndex;

	}

	void World::drawAll(){

		for(size_t i(0), maxSize(allChunk_.size()); i < maxSize; ++i){
			// std::cout << "Chunk numero : " << i << " = ";
			// allChunk_[i].describe();
			allChunk_[i].draw();
		}
		std::cout << "hey" << std::endl;
	}
	
}