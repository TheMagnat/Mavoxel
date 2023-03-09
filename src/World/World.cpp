
#include <World/World.hpp>
#include <Core/Global.hpp>

#include <iostream>
#include <chrono>

namespace mav {

	World::World(Shader* shaderPtrP, Environment* environmentP, size_t chunkSize, float voxelSize)
		: chunkSize_(chunkSize), voxelSize_(voxelSize), shader(shaderPtrP), environment(environmentP) {}

	void World::createChunk(int chunkPosX, int chunkPosY, int chunkPosZ, VoxelGeneratorFunc generator){

		size_t newChunkIndex = allChunk_.size();

		allChunk_.push_back( std::make_unique<Chunk>(this, chunkPosX, chunkPosY, chunkPosZ, chunkSize_, voxelSize_) );

		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		allChunk_.back()->generateVoxels(generator);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> fsec = end - begin;
		std::cout << "Time difference (generateVoxels) = " << fsec.count() << "s" << std::endl;
		
		begin = std::chrono::high_resolution_clock::now();
		allChunk_.back()->generateVertices();
		end = std::chrono::high_resolution_clock::now();
		fsec = end - begin;
		std::cout << "Time difference (generateVertices) = " << fsec.count() << "s" << std::endl;

		begin = std::chrono::high_resolution_clock::now();
		allChunk_.back()->graphicUpdate();
		end = std::chrono::high_resolution_clock::now();
		fsec = end - begin;
		std::cout << "Time difference (graphicUpdate) = " << fsec.count() << "s" << std::endl;

	}

	void World::createChunk(int chunkPosX, int chunkPosY, int chunkPosZ, VoxelMapGeneratorFunc voxelMapGenerator){

		size_t newChunkIndex = allChunk_.size();

		allChunk_.push_back( std::make_unique<Chunk>(this, chunkPosX, chunkPosY, chunkPosZ, chunkSize_, voxelSize_) );
		chunkCoordToIndex_[chunkPosX][chunkPosY][chunkPosZ] = newChunkIndex;

		Chunk* currentChunkPtr = allChunk_.back().get();

		auto rez = Global::threadPool.enqueue([this, currentChunkPtr, newChunkIndex, chunkPosX, chunkPosY, chunkPosZ, voxelMapGenerator](){
			
			std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
			currentChunkPtr->generateVoxels( voxelMapGenerator(chunkPosX, chunkPosY, chunkPosZ) );
			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> fsec = end - begin;
			std::cout << "Time difference (generateVoxels) = " << fsec.count() << "s" << std::endl;
			
			begin = std::chrono::high_resolution_clock::now();
			currentChunkPtr->generateVertices();
			end = std::chrono::high_resolution_clock::now();
			fsec = end - begin;
			std::cout << "Time difference (generateVertices) = " << fsec.count() << "s" << std::endl;


			currentChunkPtr->state = 1;

			// Now thread safe operation
			std::lock_guard<std::mutex> lock(readyToUpdateChunksMutex);
			readyToUpdateChunks.push(newChunkIndex);

		});

	}

	void World::updateReadyChunk(size_t nbToUpdate) {

		if (readyToUpdateChunks.empty()) return;

		std::lock_guard<std::mutex> lock(readyToUpdateChunksMutex);

		while(!readyToUpdateChunks.empty() && nbToUpdate-- != 0) {

			size_t currentChunkIndex = readyToUpdateChunks.front();
			readyToUpdateChunks.pop();

			std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

			allChunk_[currentChunkIndex]->graphicUpdate();
			allChunk_[currentChunkIndex]->state = 2;

			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> fsec = end - begin;
			std::cout << "Time difference (graphicUpdate) = " << fsec.count() << "s" << std::endl;

		}
	}

	void World::drawAll(){

		for(size_t i(0), maxSize(allChunk_.size()); i < maxSize; ++i){
			
			//TODO: Add camera vision logic to skip unecessary chunk draw

			// If chunk not ready to be drawn, skip it
			if (allChunk_[i]->state != 2) continue;

			allChunk_[i]->draw();
		}

	}
	
}