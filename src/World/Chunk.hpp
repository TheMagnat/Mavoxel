
#pragma once

#include <World/World.hpp>
#include <World/SimpleVoxel.hpp>
#include <World/Generator.hpp>
#include <GLObject/Drawable.hpp>

#include <GLObject/GLObject.hpp>

#include <vector>
#include <unordered_map>
#include <cstddef>
#include <functional>

#ifndef NDEBUG

#include <Mesh/DebugVoxel.hpp>

#endif

namespace mav {

	struct VoxelMatrix {
		std::vector<SimpleVoxel> data;
		VoxelMap voxelIndices;

		void initializeIndices(size_t size) {

			voxelIndices.resize(size);
			for (size_t x = 0; x < size; ++x) {

				voxelIndices[x].resize(size);
				for (size_t y = 0; y < size; ++y) {

					voxelIndices[x][y].resize(size, -1);
				}
			}
		
		}

	};



	class World;
	
	class Chunk : public Drawable {

		static std::array<std::pair<int, int>, 6> faceToNeighborOffset;
		static std::array<uint8_t, 6> faceToInverseFace;

		public:
			Chunk(World* world, int posX, int posY, int posZ, int size, float voxelSize);

			void generateVoxels(const VoxelMapGenerator * voxelMapGenerator);
			void generateVertices();

			/**
			 * Return 0 if nothing found, 1 if another voxel was found and 2 if out of bound.
			*/
			int findVoxel(glm::vec3 const& position, VoxelMap const& voxelMap) const;
			int findVoxel(int x, int y, int z, VoxelMap const& voxelMap) const;
			const SimpleVoxel* unsafeGetVoxel(int x, int y, int z) const;

			//OpenGL
			void draw();

			//Current GL state of the chunk. 0 mean data not ready, 1 mean data ready but VAO not up-to-date, 2 mean data and VAO ready.
			int state;
			
		public:
			int posX_;
			int posY_;
			int posZ_;

			int size_;
			float voxelSize_;

			AABB collisionBox_;

			//Voxels informations
			VoxelMatrix voxels_;

			//Reference to the world
			World* world_;


			#ifndef NDEBUG
				DebugVoxel chunkSides;
    		#endif
	};

}
