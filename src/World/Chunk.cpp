
#include <World/Chunk.hpp>
#include <GLObject/BufferTemplates.hpp>

#include <iostream>

#ifdef TIME
//Help/debug
#include <Helper/Benchmark/Profiler.hpp>
#endif

#ifndef NDEBUG
#include <Core/DebugGlobal.hpp>
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

	
	Chunk::Chunk(World* world, int posX, int posY, int posZ, size_t octreeDepth, float voxelSize, const VoxelMapGenerator * voxelMapGenerator)
		: state(0), posX_(posX), posY_(posY), posZ_(posZ),
		size_((int)svo_.getLen()), voxelSize_(voxelSize),
		centerWorldPosition_(posX*(size_*voxelSize), posY*(size_*voxelSize), posZ*(size_*voxelSize)),
		collisionBox_(glm::vec3(centerWorldPosition_), size_*voxelSize),
		svo_(octreeDepth),
		world_(world), voxelMapGenerator_(voxelMapGenerator)

		{}


	void Chunk::generateVoxels() {
		
		#ifdef TIME
			Profiler profiler("Generate voxels");
		#endif

		// Move voxelMap and prepare indices size
		VoxelData voxelData = voxelMapGenerator_->generate(posX_, posY_, posZ_);
		VoxelMap const& voxelMap = voxelData.map;

		// Calculate the coordinates of each points
		for (size_t x = 0; x < size_; ++x) {
			for (size_t y = 0; y < size_; ++y) {
				for (size_t z = 0; z < size_; ++z) {
					
					if( voxelMap[x][y][z] == 0 ) continue;

					svo_.set(glm::uvec3(x, y, z), voxelMap[x][y][z]);

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

		return svo_.get(position).first != 0;
	}

	int Chunk::findVoxel(int x, int y, int z) const {

		if (x < 0 || y < 0 || z < 0 || x >= size_ || y >= size_ || z >= size_) return 2;

		return svo_.get(glm::uvec3(x, y, z)).first != 0;
	}

	int32_t Chunk::unsafeGetVoxel(int x, int y, int z) {

		return svo_.get(glm::uvec3(x, y, z)).first;

	}

	int32_t Chunk::findAndGetVoxelIndex(glm::ivec3 const& position) {
		if (position.x < 0 || position.y < 0 || position.z < 0 || position.x >= size_ || position.y >= size_ || position.z >= size_) return -2;

		return svo_.get(position).first;
	}

	std::pair<int32_t, Chunk*> Chunk::getSideVoxel(glm::ivec3 voxelPosition, uint8_t side) {

		#ifdef TIME
			Profiler profiler("Get side voxel");
		#endif

		glm::ivec3 neighborPosition = {
				voxelPosition.x + (Chunk::faceToNeighborOffset[side].first == 0) * Chunk::faceToNeighborOffset[side].second,
				voxelPosition.y + (Chunk::faceToNeighborOffset[side].first == 1) * Chunk::faceToNeighborOffset[side].second,
				voxelPosition.z + (Chunk::faceToNeighborOffset[side].first == 2) * Chunk::faceToNeighborOffset[side].second
		};

		//Verify if voxel is in current chunk or another chunk
		int32_t sideVoxelIndex = findAndGetVoxelIndex(neighborPosition);
		if (sideVoxelIndex >= 0) {
			return {sideVoxelIndex, nullptr};
		}
		else if (sideVoxelIndex == -2) {

			Chunk* sideChunk = world_->getChunk(
				posX_ + (Chunk::faceToNeighborOffset[side].first == 0) * Chunk::faceToNeighborOffset[side].second,
				posY_ + (Chunk::faceToNeighborOffset[side].first == 1) * Chunk::faceToNeighborOffset[side].second,
				posZ_ + (Chunk::faceToNeighborOffset[side].first == 2) * Chunk::faceToNeighborOffset[side].second
			);

			if (sideChunk == nullptr) {
				//Special case, the position is not in the current chunk but the side voxel does not exist
				return {-1, nullptr};
			}

			neighborPosition[Chunk::faceToNeighborOffset[side].first] -= size_ * Chunk::faceToNeighborOffset[side].second;
			int32_t sideChunkVoxelIndex = sideChunk->findAndGetVoxelIndex(neighborPosition);

			if (sideChunkVoxelIndex >= 0) {
				return {sideChunkVoxelIndex, sideChunk};
			}
			else {
				return {-1, sideChunk};
			}

		}

		//Here, the position is in the current chunk but the voxel is empty
		return {-1, nullptr};
	}

	bool Chunk::deleteVoxel(glm::ivec3 position) {

		#ifdef TIME
			Profiler profiler("Delete voxel");
		#endif

		svo_.set(position, 0);

		//We notify that we need to regenerate this chunk
		world_->needToRegenerateChunks.insert(this);

		return true;
	}

	glm::ivec3 Chunk::getPosition() const {
		return glm::ivec3(posX_, posY_, posZ_);
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

		//In this case, we need to insert it in the neighbor chunk
		if (position.x < 0 || position.y < 0 || position.z < 0 || position.x >= size_ || position.y >= size_ || position.z >= size_) {
			affectedChunk = getChunk(position);
		}

		if (affectedChunk) affectedChunk->unsafeAddVoxel(position, newVoxelId);

	}

	void Chunk::unsafeAddVoxel(glm::ivec3 position, int newVoxelId) {

		svo_.set(position, newVoxelId);

		//Notify that we need to regenerate this chunk
		world_->needToRegenerateChunks.insert(this);

	}

	void Chunk::graphicUpdate() {		
		#ifdef RAY_CAST
		svo_.updateBuffer();
		#endif
	}
	
}