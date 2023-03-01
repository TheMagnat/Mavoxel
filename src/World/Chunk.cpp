
#include "World/Chunk.hpp"

#include <iostream>

namespace mav{
	
	Chunk::Chunk(World* world, int posX, int posZ, size_t size, size_t voxelSize)
		: posX_(posX), posZ_(posZ), size_(size), voxelSize_(voxelSize), world_(world) {}

	void Chunk::generate(){

		Material grassMaterial {
			{0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.5f, 0.5f, 0.5f},
			32.0f
		};

		for (size_t x = 0; x < size_; ++x)
		{
			for (size_t z = 0; z < size_; ++z)
			{

				float xPos = x*voxelSize_;
				float zPos = z*voxelSize_;
				float yPos = (x+z)*voxelSize_;

				data_.emplace_back(world_->shaderPtr, world_->environment, grassMaterial, voxelSize_, glm::vec3(xPos, yPos, zPos));
				data_.back().init();
			}
			
		}

	}

	void Chunk::describe(){
		std::cout << "Je suis un chunk de taille : " << size_ << std::endl;
	}

	void Chunk::draw(){

		for (size_t i = 0; i < data_.size(); ++i)
		{
			data_[i].draw();
		}

	}

}