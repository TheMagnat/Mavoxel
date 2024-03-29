
#pragma once

#include <World/World.hpp>
#include <World/SimpleVoxel.hpp>
#include <World/Generator.hpp>

#include <Octree/SparseVoxelOctree.hpp>

#include <vector>
#include <unordered_map>
#include <cstddef>
#include <functional>

#ifndef NDEBUG

#include <Mesh/DebugVoxel.hpp>

#endif


namespace mav {

	class World;
	
	class Chunk {

		static std::array<std::pair<int, int>, 6> faceToNeighborOffset;
		static std::array<uint8_t, 6> faceToInverseFace;

		public:
			Chunk(World* world, glm::ivec3 const& position, size_t octreeDepth, float voxelSize);

			void generateVoxels(const VoxelMapGenerator * voxelMapGenerator);
			void graphicUpdate();

			//File saving
			void loadVoxels(std::ifstream* stream);
			void saveVoxels(std::ofstream* stream);


			//Find and/or get a voxel

			/**
			 * Return 0 if nothing found, 1 if another voxel was found and 2 if out of bound.
			 * Their is a glm::vec3 version and one with 3 int for each axis.
			 * 
			 * Their is also a version using a Voxel Map and a version using directly the stored VoxelMatrix.
			*/
			int findVoxel(glm::vec3 const& position, VoxelMap const& voxelMap) const;
			int findVoxel(int x, int y, int z, VoxelMap const& voxelMap) const;
			int findVoxel(glm::ivec3 const& position) const;
			int findVoxel(int x, int y, int z) const;

			/**
			 * @brief Get the voxel at the given position.
			 * @note This method will not verify if the given position is in the chunk range.
			 * 
			 * @param x 
			 * @param y 
			 * @param z 
			 * TODO: refaire description
			 * @return SimpleVoxel* A Pointer to the requested voxel. nullptr will be returned if the position is empty.
			 */
			int32_t unsafeGetVoxel(int x, int y, int z) const;

			/**
			 * Return -2 if position is out of bound, -1 if the position is empty
			 * or the index of the voxel at the given position.
			*/
			int32_t findAndGetVoxelIndex(glm::ivec3 const& position);

			/**
			 * @brief Delete a voxel at the given position.
			 * @note This method will also populate "needToRegenerateChunks" from the world_ pointer
			 * to notify the chunk that need to have their vertices regenerated (including this chunk
			 * and the neighbors chunks of the deleted voxel).
			 * 
			 * @param position The chunk position of he deleted voxel
			 * @return true if a voxel was present and got deleted
			 * @return false otherwise
			 */
			bool deleteVoxel(glm::ivec3 position);

			/**
			 * @brief Add a voxel at the given chunk position. The position can lead to another chunk.
			 * @note This method will then call the unsafeAddVoxel of the corresponding chunk.
			 * 
			 * @param position Chunk position of the voxel to add
			 * @param newVoxelId Id of the added voxel
			 */
			void addVoxel(glm::ivec3 position, int newVoxelId);

			/**
			 * @brief Add a voxel at the given chunk position.
			 * @note If a voxel already exist at the given position, it will be replaced.
			 * This method will also populate "needToRegenerateChunks" from the world_ pointer.
			 * 
			 * @param position Chunk position of the voxel to add. It must be in the chunk range ([0 - size_[ for each axis)
			 * @param newVoxelId Id of the added voxel
			 */
			void unsafeAddVoxel(glm::ivec3 position, int newVoxelId);

			/**
			 * Return a pair containing a pointer to the side voxel of the position if it exist or nullptr otherwise
			 * and a pointer to the side chunk if the voxel is not in the current chunk, or nullptr otherwise.
			*/
			std::pair<int32_t, Chunk*> getSideVoxel(glm::ivec3 position, uint8_t side);

			/**
			 * Get the real chunk from the given voxel chunk position.
			 * This method will also adapt voxelChunkPosition to the position of the returned chunk.
			*/
			Chunk* getChunk(glm::ivec3& voxelChunkPosition) const;

			glm::ivec3 const& getPosition() const;

			//Vulkan
			void draw(VkCommandBuffer commandBuffer);
			void debugDraw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

			//Current GL state of the chunk. 0 mean data not ready, 1 mean data ready but VAO not up-to-date, 2 mean data and VAO ready.
			int state;
		//TODO: private
		public:

			//Voxels informations
			SparseVoxelOctree svo_; //SVO must be initialized first to then get it's len

			glm::ivec3 position_;

			int size_; //Note: we store size as an int because size_t could cause problem with negativity
			float voxelSize_;
			glm::vec3 centerWorldPosition_;

			AABB collisionBox_;

			//Reference to the world
			World* world_;

	};

}
