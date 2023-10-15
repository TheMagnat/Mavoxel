#pragma once

#include <World/ChunkIndexer.hpp>

#include <World/SimpleVoxel.hpp>
#include <World/Chunk.hpp>
#include <World/Generator.hpp>

#include <VulkanWrapper/Shader.hpp>
#include <Environment/Environment.hpp>
#include <Helper/ThreadPool.hpp>
#include <Collision/AABB.hpp>
#include <Collision/CollisionFace.hpp>


#include <vector>
#include <queue>
#include <unordered_map>
#include <set>
#include <mutex>
#include <memory>
#include <optional>

namespace mav {

	class Chunk;
	struct RayCollisionInformations;

	class World {

		public:
			World(const VoxelMapGenerator* voxelMapGenerator, size_t octreeDepth = 5, float voxelSize = 1.0f, std::string const& worldFolderPath = "", bool newWorld = true);

			std::vector<glm::ivec3> getAroundChunks(glm::vec3 const& position, float distance, bool sorted) const;
			
			/**
			 * Try to load a Chunk from Chunk indexer.
			 * If it does not exist, create it using the voxel map generator.
			*/
			void loadOrCreateChunk(glm::ivec3 const& chunkPosition);
			void createChunk(glm::ivec3 const& chunkPosition);
			void loadChunk(glm::ivec3 const& chunkPosition, size_t fileIndex);

			void save();

			void bulkCreateChunk(glm::vec3 const& position, float createDistance, bool sorted);

			Chunk* getChunk(int x, int y, int z);
			Chunk* getChunk(glm::ivec3 const& position);
			const Chunk* getChunk(glm::ivec3 const& position) const;
			
			Chunk* getChunkFromWorldPos(glm::vec3 const& position);
			
			inline glm::ivec3 getChunkIndex(glm::vec3 const& position) const { return glm::floor(position / (chunkSize_ * voxelSize_)); }
			//void drawAll(VkCommandBuffer currentCommandBuffer, uint32_t currentFrame);

			//TODO: Save entre les draw la position de l'utilisateur et garder en mémoir les chunks a draw, et update ce vecteur si l'utilisateur à bougé
			//void draw(glm::vec3 position, float renderDistance);

			// Collisions

			/**
			 * @brief Find if there is a voxel at the given position.
			 * 
			 * @param x 
			 * @param y 
			 * @param z 
			 * @return Return a pair containing pointer to the found voxel and a pointer to the corresponding chunk if found,
			 * or a pair of nullptr if not.
			 */
			std::tuple<int32_t, const Chunk*, glm::uvec3> getVoxel(float x, float y, float z) const;

			/**
			 * @brief Cast a ray in the world to find the face encountered.
			 * 
			 * @param startPosition The position where we will start to cast our ray.
			 * @param direction The direction of the ray.
			 * @param maxDistance The maximum distance that we will travel before considering there is no collision.
			 * @return We will return an optional CollisionFace which is filled only if we found a collision.
			 */

			//TODO: changer le retour ?
			//TODO: Refacto pour n'avoir plus qu'une fonction ?
			std::optional<RayCollisionInformations> castRay(glm::vec3 position, glm::vec3 const& inputDirection, float maxDistance = 50);
			std::optional<RayCollisionInformations> castRay(glm::vec3 position, glm::vec3 const& inputDirection, float maxDistance = 50) const;

			// std::optional<RayCollisionInformations> castSVORay(glm::vec3 const& startPosition, glm::vec3 const& direction, float maxDistance = 50) const;


			/**
			 * @brief Verify if a moving bounding box enter in collision with our world.
			 * 
			 * @param box The bounding box that will be tested in our world
			 * @param direction The direction of the box.
			 * @return A pair of glm::vec3. The first element represent the newly calculated velocity with the collisions applied to it
			 * and the second contain the informations about the axis where collision where encountered.
			 */
			std::pair<glm::vec3, glm::vec3> collide(mav::AABB const& box, glm::vec3 direction) const;

			size_t getChunkSize() const;
			float getVoxelSize() const;

			// Threaded
			void updateReadyChunk(size_t nbToUpdate = 1);

		private:
			const VoxelMapGenerator * voxelMapGenerator_;

			size_t octreeDepth_;
			size_t chunkSize_;
			float voxelSize_;

			//std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, size_t>>> chunkCoordToIndex_;
			std::unordered_map<glm::ivec3, Chunk> chunks_;


			std::mutex readyToUpdateChunksMutex;
			std::queue<Chunk*> readyToUpdateChunks;

		public:
			//World generation data
			Environment* environment;

			//Material material;

			//Threading
			ThreadPool threadPool; //Note that destruction is done in reverse order than construction. So threadPool will be destructed first, preventing error when joining threads.
			
			//Chunk that need to have their vertices regenerated
			std::set<Chunk*> needToRegenerateChunks;


			//Save informations
			ChunkIndexer chunkIndexer_;
			bool fileSaving_;
			std::string worldFolderPath_;

	};

}
