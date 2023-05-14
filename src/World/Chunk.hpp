
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

#ifdef RAY_CAST
#include <GLObject/Texture3D.hpp>
#endif

namespace mav {

	struct VoxelMatrix {
		std::vector<SimpleVoxel> data;
		VoxelMap voxelIndices;

		#ifdef RAY_CAST
		std::vector<uint8_t> voxelMatrix;

		void fillMatrix(VoxelMap const& matrix) {

			size_t chunkSize = matrix.size();

			voxelMatrix.clear();
            voxelMatrix.reserve(chunkSize * chunkSize * chunkSize);

			for (size_t z = 0; z < chunkSize; ++z) {
				for (size_t y = 0; y < chunkSize; ++y) {
					for (size_t x = 0; x < chunkSize; ++x) {
						voxelMatrix.push_back( matrix[x][y][z] );
					}
				}
			}

		}

		#endif

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
			Chunk(World* world, int posX, int posY, int posZ, size_t size, float voxelSize, const VoxelMapGenerator * voxelMapGenerator);

			void generateVoxels();
			void generateVertices();

			std::array<float, 4> generateAmbientOcclusion(SimpleVoxel const& voxel, uint8_t faceIndex) const;


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
			 * @return SimpleVoxel* A Pointer to the requested voxel. nullptr will be returned if the position is empty.
			 */
			SimpleVoxel* unsafeGetVoxel(int x, int y, int z);

			/**
			 * Return -2 if position is out of bound, -1 if the position is empty
			 * or the index of the voxel at the given position.
			*/
			int findAndGetVoxelIndex(glm::ivec3 const& position);

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
			std::pair<SimpleVoxel*, Chunk*> getSideVoxel(glm::ivec3 position, uint8_t side);

			/**
			 * Get the real chunk from the given voxel chunk position.
			 * This method will also adapt voxelChunkPosition to the position of the returned chunk.
			*/
			Chunk* getChunk(glm::ivec3& voxelChunkPosition) const;

			#ifdef RAY_CAST
			const Texture3D* getTexture() const {
				return &texture;
			}
			#endif

			//Vulkan
			void updateTexture();
			void draw(VkCommandBuffer commandBuffer);
			void debugDraw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

			std::vector<uint32_t> getVertexAttributesSizes() const override;

			void updateUniforms(vuw::Shader* shader, uint32_t currentFrame) const override;

			//Current GL state of the chunk. 0 mean data not ready, 1 mean data ready but VAO not up-to-date, 2 mean data and VAO ready.
			int state;
			
		public:
			int posX_;
			int posY_;
			int posZ_;

			int size_; //Note: we store size as an int because size_t could cause problem with negativity
			float voxelSize_;
			float positionOffsets_;
			glm::vec3 centerWorldPosition_;

			AABB collisionBox_;

			//Voxels informations
			VoxelMatrix voxels_;

			#ifdef RAY_CAST
			Texture3D texture;
			#endif


			//Reference to the world
			World* world_;
			const VoxelMapGenerator * voxelMapGenerator_;


			#ifndef NDEBUG
				DebugVoxel chunkSides;
    		#endif
	};

}
