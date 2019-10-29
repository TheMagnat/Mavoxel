#pragma once

#include "Chunk.hpp"

#include <vector>
#include <unordered_map>

namespace mav{

	class World{

		public:
			World(size_t chunkSize = 32);

			void createChunk(int chunkPosX, int chunkPosY);

			void drawAll();
		
		private:
			size_t chunkSize_;

			std::vector<Chunk> allChunk_;

			std::unordered_map<int, std::unordered_map<int, size_t>> chunkCoordToIndex_;

	};

}