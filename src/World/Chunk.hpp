
#pragma once

#include <World/World.hpp>
#include <World/SimpleVoxel.hpp>

#include <GLObject/GLObject.hpp>

#include <vector>
#include <unordered_map>
#include <cstddef>

namespace mav{

	class World;

	
	class Chunk{

		public:
			Chunk(World* world, int posX, int posZ, size_t size, size_t voxelSize); //TODO: Add Y

			void generateVoxels();
			void generateVertices();

			void describe();

			//OpenGL
			void graphicUpdate();
			void draw();
			
		private:
			int posX_;
			int posZ_;
			size_t size_;
			size_t voxelSize_;

			//Voxels informations
			std::vector<SimpleVoxel> voxels_;
			std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, size_t>>> voxelCoordToIndex_;

			//OpenGL
			VAO vao_;
            std::vector<float> vertices_;

            size_t indicesNb_;
            std::vector<int> indices_;

			//Reference to the world
			World* world_;
	};

}
