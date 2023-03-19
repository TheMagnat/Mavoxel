
#include <World/World.hpp>
#include <Core/Global.hpp>

#include <iostream>
#include <chrono>

namespace mav {

	World::World(Shader* shaderPtrP, Environment* environmentP, size_t chunkSize, float voxelSize)
		: chunkSize_(chunkSize), voxelSize_(voxelSize), shader(shaderPtrP), environment(environmentP) {}

	void World::createChunk(int chunkPosX, int chunkPosY, int chunkPosZ, const VoxelMapGenerator * voxelMapGenerator){

		size_t newChunkIndex = allChunk_.size();

		allChunk_.push_back( std::make_unique<Chunk>(this, chunkPosX, chunkPosY, chunkPosZ, chunkSize_, voxelSize_) );
		chunkCoordToIndex_.emplace(ChunkCoordinates(chunkPosX, chunkPosY, chunkPosZ), newChunkIndex);

		Chunk* currentChunkPtr = allChunk_.back().get();

		auto rez = Global::threadPool.enqueue([this, currentChunkPtr, newChunkIndex, chunkPosX, chunkPosY, chunkPosZ, voxelMapGenerator](){
			
			std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
			currentChunkPtr->generateVoxels( voxelMapGenerator );
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

	const SimpleVoxel* World::getVoxel(float x, float y, float z) const {

		float xSign = x < 0 ? -1 : 1;
		float ySign = y < 0 ? -1 : 1;
		float zSign = z < 0 ? -1 : 1;

		float chunkLen = (float)(chunkSize_ - 1);

		float halfChunkSize = chunkLen / 2.0f;

		int xIndex = (x + halfChunkSize * xSign) / (float)chunkSize_;
		int yIndex = (y + halfChunkSize * ySign) / (float)chunkSize_;
		int zIndex = (z + halfChunkSize * zSign) / (float)chunkSize_;

		auto chunkIt = chunkCoordToIndex_.find(ChunkCoordinates(xIndex, yIndex, zIndex));
		if (chunkIt == chunkCoordToIndex_.end()) return nullptr; //The chunk does not exist
		if (allChunk_[chunkIt->second]->state != 2) return nullptr;
		//TODO: We could return something else to make the user understand the chunk does not exist and make it jump directly to the next chunk

		int internalX = (x + halfChunkSize) - xIndex * (int)chunkSize_;
		int internalY = (y + halfChunkSize) - yIndex * (int)chunkSize_;
		int internalZ = (z + halfChunkSize) - zIndex * (int)chunkSize_;

		return allChunk_[chunkIt->second]->unsafeGetVoxel(internalX, internalY, internalZ);

	}


	inline float positiveModulo (float a, float b) { return a >= 0 ? fmod(a, b) : fmod( fmod(a, b) + b, b); }

	const SimpleVoxel* World::CastRay(glm::vec3 startPosition, glm::vec3 direction, size_t maxNumberOfVoxels) const {

		glm::vec3 dir = glm::normalize(direction);

		//Index of the first voxel
		float x = floor(startPosition.x / voxelSize_) * voxelSize_;
		float y = floor(startPosition.y / voxelSize_) * voxelSize_;
		float z = floor(startPosition.z / voxelSize_) * voxelSize_;

		//TODO: Checker le pb de carré blanc qui s'affiche pas si on set met vers le haut d'un chunk et qu'on vise une case au dessus en regardant tout droit

		//Positive or negative direction
		float xStep = dir.x < 0 ? -1 : 1;
		float yStep = dir.y < 0 ? -1 : 1;
		float zStep = dir.z < 0 ? -1 : 1;

		//Required time to exit the full voxel along each axis.
		float tMaxX = positiveModulo(startPosition.x, voxelSize_);
		float tMaxY = positiveModulo(startPosition.y, voxelSize_);
		float tMaxZ = positiveModulo(startPosition.z, voxelSize_);
		
		if (xStep == 1) tMaxX = voxelSize_ - tMaxX;
		if (yStep == 1) tMaxY = voxelSize_ - tMaxY;
		if (zStep == 1) tMaxZ = voxelSize_ - tMaxZ;

		tMaxX /= dir.x * xStep;
		tMaxY /= dir.y * yStep;
		tMaxZ /= dir.z * zStep;

		//Required time to do a full voxel length along the axis.
		float tDeltaX = voxelSize_ / dir.x * xStep;
		float tDeltaY = voxelSize_ / dir.y * yStep;
		float tDeltaZ = voxelSize_ / dir.z * zStep;

		//TODO: Maybe we could remove this or maybe bot
		const SimpleVoxel* foundVoxel = getVoxel(x + (voxelSize_ / 2.0f) * xStep, y + (voxelSize_ / 2.0f) * yStep, z + (voxelSize_ / 2.0f) * zStep);
		for(size_t i = 0; foundVoxel == nullptr && i < maxNumberOfVoxels; ++i) {

			if(tMaxX < tMaxY) {

				if (tMaxX < tMaxZ) {

					tMaxX = tMaxX + tDeltaX;
					x = x + xStep * voxelSize_;

				}
				else {

					tMaxZ = tMaxZ + tDeltaZ;
					z = z + zStep * voxelSize_;

				}

			}
			else {

				if (tMaxY < tMaxZ) {

					tMaxY = tMaxY + tDeltaY;
					y = y + yStep * voxelSize_;

				}
				else {
					
					tMaxZ = tMaxZ + tDeltaZ;
					z = z + zStep * voxelSize_;

				}

			}

			//We add half the voxel size to center it.
			foundVoxel = getVoxel(x + (voxelSize_ / 2.0f), y + (voxelSize_ / 2.0f), z + (voxelSize_ / 2.0f));

		}

		return foundVoxel;

	}
	
}