
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

	struct VoxelMatrice {
		std::vector<SimpleVoxel> data;
		VoxelMap voxelIndices;
		VoxelMap voxelMap;

		void initializeIndices(size_t size) {

			voxelIndices.resize(size);
			for (size_t x = 0; x < size; ++x) {

				voxelIndices[x].resize(size);
				for (size_t y = 0; y < size; ++y) {

					voxelIndices[x][y].resize(size, -1);
				}
			}
		
		}

		void initializeMap(size_t size) {

			voxelMap.resize(size);
			for (size_t x = 0; x < size; ++x) {

				voxelMap[x].resize(size);
				for (size_t y = 0; y < size; ++y) {

					voxelMap[x][y].resize(size);
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

			void generateVoxels(VoxelGeneratorFunc generator);
			void generateVoxels(const VoxelMapGenerator * voxelMapGenerator);
			void generateVertices();

			/**
			 * Reeturn 0 if nothing found, 1 if another voxel was found and 2 if out of bound.
			*/
			int findVoxel(glm::vec3 const& position) const;
			int findVoxel(int x, int y, int z) const;
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

			//Voxels informations
			VoxelMatrice voxels_;

			//Reference to the world
			World* world_;


			#ifndef NDEBUG

				DebugVoxel chunkSides;

    		#endif
	};

}
