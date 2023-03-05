
#include "World/World.hpp"

#include <iostream>

namespace mav {

	World::World(Shader* shaderPtrP, Environment* environmentP, size_t chunkSize, size_t voxelSize)
		: chunkSize_(chunkSize), voxelSize_(voxelSize), shader(shaderPtrP), environment(environmentP) {}

	void World::createChunk(int chunkPosX, int chunkPosY, int chunkPosZ){

		size_t newChunkIndex = allChunk_.size();

		allChunk_.emplace_back(this, chunkPosX, chunkPosZ, chunkSize_, voxelSize_);
		allChunk_.back().generateVoxels();
		allChunk_.back().generateVertices();
		allChunk_.back().graphicUpdate();

		chunkCoordToIndex_[chunkPosX][chunkPosY][chunkPosZ] = newChunkIndex;

	}

	void World::drawAll(){

		for(size_t i(0), maxSize(allChunk_.size()); i < maxSize; ++i){
			// std::cout << "Chunk numero : " << i << " = ";
			// allChunk_[i].describe();
			allChunk_[i].draw();
		}

	}
	
}