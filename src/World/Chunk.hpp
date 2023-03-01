
#pragma once

#include <World/World.hpp>
#include <Mesh/Voxel.hpp>

#include <vector>
#include <cstddef>

namespace mav{

	class World;

	
	class Chunk{

		public:
			Chunk(World* world, int posX, int posZ, size_t size, size_t voxelSize);

			void generate();

			void describe();

			void draw();
			
		private:
			int posX_;
			int posZ_;
			size_t size_;
			size_t voxelSize_;

			std::vector<Voxel> data_;

			World* world_;
	};

}