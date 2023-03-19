#pragma once

#include <World/SimpleVoxel.hpp>
#include <World/Chunk.hpp>
#include <World/Generator.hpp>
#include <World/CoordinatesHelper.hpp>


#include <GLObject/Shader.hpp>
#include <Environment/Environment.hpp>

#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace mav {

	class Chunk;

	class World {

		public:
			World(Shader* shaderPtrP, Environment* environmentP, size_t chunkSize = 32, float voxelSize = 1);

			void createChunk(int chunkPosX, int chunkPosY, int chunkPosZ, const VoxelMapGenerator * voxelMapGenerator);

			void drawAll();

			// Collisions
			const SimpleVoxel* getVoxel(float x, float y, float z) const;
			const SimpleVoxel* CastRay(glm::vec3 startPosition, glm::vec3 direction, size_t maxNumberOfVoxels = 50) const;


			// Threaded
			void updateReadyChunk(size_t nbToUpdate = 1);

		private:
			size_t chunkSize_;
			float voxelSize_;

			std::vector<std::unique_ptr<Chunk>> allChunk_;

			//std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, size_t>>> chunkCoordToIndex_;
			std::unordered_map<ChunkCoordinates, size_t> chunkCoordToIndex_;


			std::mutex readyToUpdateChunksMutex;
			std::queue<size_t> readyToUpdateChunks;

		public:
			//World generation data
			Shader* shader;
			Environment* environment;
			//Material material;
			
	};

}