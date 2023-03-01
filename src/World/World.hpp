#pragma once

#include <World/Chunk.hpp>

#include <GLObject/Shader.hpp>
#include <Environment/Environment.hpp>

#include <vector>
#include <unordered_map>

namespace mav{

	class Chunk;


	class World {

		public:
			World(Shader* shaderPtrP, Environment* environmentP, size_t chunkSize = 32, size_t voxelSize = 1);

			void createChunk(int chunkPosX, int chunkPosZ);

			void drawAll();
		
		private:
			size_t chunkSize_;
			size_t voxelSize_;

			std::vector<Chunk> allChunk_;

			std::unordered_map<int, std::unordered_map<int, size_t>> chunkCoordToIndex_;

		public:
			//World generation data
			Shader* shaderPtr;
			Environment* environment;
			//Material material;
			
	};

}