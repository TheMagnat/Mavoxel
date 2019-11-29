
#include "World/World.hpp"

#include <iostream>

namespace mav {

	World::World(size_t chunkSize) : chunkSize_(chunkSize) {



	}

	void World::createChunk(int chunkPosX, int chunkPosY){

		size_t newIndex = allChunk_.size();

		allChunk_.emplace_back(chunkSize_);

		chunkCoordToIndex_[chunkPosX][chunkPosY] = newIndex;

	}

	void World::drawAll(){

		for(size_t i(0), maxSize(allChunk_.size()); i < maxSize; ++i){
			std::cout << "Chunk numero : " << i << " = ";
			allChunk_[i].describe();
		}

	}
	
}