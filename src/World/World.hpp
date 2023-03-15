#pragma once

#include <World/Chunk.hpp>
#include <World/Generator.hpp>

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

			//TODO: Ne plus utiliser que celle avec le generator mais s'en servir pour générer la VoxelMap
			void createChunk(int chunkPosX, int chunkPosY, int chunkPosZ, VoxelGeneratorFunc generator);
			void createChunk(int chunkPosX, int chunkPosY, int chunkPosZ, const VoxelMapGenerator * voxelMapGenerator);

			void drawAll();

			// Threaded
			void updateReadyChunk(size_t nbToUpdate = 1);

		private:
			size_t chunkSize_;
			float voxelSize_;

			std::vector<std::unique_ptr<Chunk>> allChunk_;

			std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, size_t>>> chunkCoordToIndex_;


			std::mutex readyToUpdateChunksMutex;
			std::queue<size_t> readyToUpdateChunks;

		public:
			//World generation data
			Shader* shader;
			Environment* environment;
			//Material material;
			
	};

}