
#include <World/World.hpp>
#include <World/WorldPositionHelper.hpp>
#include <Core/Global.hpp>


#include <iostream>
#include <chrono>

#include <algorithm>

#ifndef NDEBUG
#include <Core/DebugGlobal.hpp>
#endif

#ifdef TIME
#include <Helper/Benchmark/Profiler.hpp>
#endif



namespace mav {

	World::World(size_t octreeDepth, float voxelSize)
		//TODO: Rendre le nombre de thread paramétrable
		: octreeDepth_(octreeDepth), chunkSize_(std::pow(2, octreeDepth)), voxelSize_(voxelSize), threadPool(8)
		{
			//TODO: Think of a better way to prevent having to put a mutex in getChunk... Maybe storing directly the unique_ptr in the map ?
			allChunk_.reserve(10000);
		}

	void World::createChunk(int chunkPosX, int chunkPosY, int chunkPosZ, const VoxelMapGenerator * voxelMapGenerator){

		size_t newChunkIndex = allChunk_.size();

		allChunk_.push_back( std::make_unique<Chunk>(this, chunkPosX, chunkPosY, chunkPosZ, octreeDepth_, voxelSize_, voxelMapGenerator) );
		chunkCoordToIndex_.emplace(ChunkCoordinates(chunkPosX, chunkPosY, chunkPosZ), newChunkIndex);

		Chunk* currentChunkPtr = allChunk_.back().get();

		//TODO: faire un truc du rez ou changer la classe threadPool pour ne plus rien renvoyer
		auto rez = threadPool.enqueue([this, currentChunkPtr, newChunkIndex](){
			
			#ifdef TIME
				Profiler profiler("Full chunks generation");
			#endif

			currentChunkPtr->generateVoxels();
			currentChunkPtr->state = 1;

			// Now thread safe operation
			std::lock_guard<std::mutex> lock(readyToUpdateChunksMutex);
			readyToUpdateChunks.push(newChunkIndex);

		});

	}

	void World::bulkCreateChunk(glm::vec3 position, float createDistance, bool sorted, const VoxelMapGenerator * voxelMapGenerator) {

		for (glm::vec3 const& chunkPosition : getAroundChunks(position, createDistance, sorted)) {

			//Try finding the chunk
			auto chunkIt = chunkCoordToIndex_.find(ChunkCoordinates(chunkPosition.x, chunkPosition.y, chunkPosition.z));
			if (chunkIt != chunkCoordToIndex_.end()) continue;
			
			//If it does not exist, we create it
			createChunk(chunkPosition.x, chunkPosition.y, chunkPosition.z, voxelMapGenerator);

		}

	}

	Chunk* World::getChunk(int x, int y, int z) {
		auto chunkIt = chunkCoordToIndex_.find(ChunkCoordinates(x, y, z));
		if (chunkIt == chunkCoordToIndex_.end()) return nullptr;
		
		Chunk* foundChunk = allChunk_[chunkIt->second].get();
		if (foundChunk->state == 0) return nullptr;

		return foundChunk;
	}

	Chunk* World::getChunkFromWorldPos(glm::vec3 position) {

		//Calculate center position		
		float trueChunkSize = chunkSize_ * voxelSize_;

		int xIndex = floor(position.x / trueChunkSize);
		int yIndex = floor(position.y / trueChunkSize);
		int zIndex = floor(position.z / trueChunkSize);

		auto chunkIt = chunkCoordToIndex_.find(ChunkCoordinates(xIndex, yIndex, zIndex));
		if (chunkIt == chunkCoordToIndex_.end()) return nullptr;
		
		Chunk* foundChunk = allChunk_[chunkIt->second].get();
		if (foundChunk->state == 0) return nullptr;

		return foundChunk;
	}

	glm::ivec3 World::getChunkIndex(glm::vec3 position) const {
		
		//Calculate center position		
		float trueChunkSize = chunkSize_ * voxelSize_;

		glm::ivec3 ret = glm::floor(position / trueChunkSize);

		return ret;

	}

	std::vector<glm::vec3> World::getAroundChunks(glm::vec3 position, float distance, bool sorted) const {
		
		std::vector<glm::vec3> aroundChunks;

		//Calculate center position
		float xSign = position.x < 0 ? -1 : 1;
		float ySign = position.y < 0 ? -1 : 1;
		float zSign = position.z < 0 ? -1 : 1;
		
		float trueChunkSize = chunkSize_ * voxelSize_;

		int xIndex = std::floor(position.x / trueChunkSize);
		int yIndex = std::floor(position.y / trueChunkSize);
		int zIndex = std::floor(position.z / trueChunkSize);

		//Calculate the number of chunk to render
		int nb_chunk = distance / trueChunkSize;

		
		for (int x = -nb_chunk, xState = 1; x <= nb_chunk; ++x) {
			for (int y = -nb_chunk, yState = 1; y <= nb_chunk; ++y) {
				for (int z = -nb_chunk, zState = 1; z <= nb_chunk; ++z) {
					aroundChunks.emplace_back(x + xIndex, y + yIndex, z + zIndex);
				}
			}
		}

		if (sorted) {

			glm::vec3 center(xIndex, yIndex, zIndex);

			// Sort the coordinates vector to have the nearest to the position first
			std::sort(aroundChunks.begin(), aroundChunks.end(),
				[&center](glm::vec3 const& coordA, glm::vec3 const& coordB) -> bool {
					return glm::distance(coordA, center) < glm::distance(coordB, center);
				}
			);
		}

		return aroundChunks;
	}

	void World::updateReadyChunk(size_t nbToUpdate) {

		if (readyToUpdateChunks.empty()) return;

		#ifdef TIME
			Profiler profiler("Graphic update (full)");
		#endif

		std::lock_guard<std::mutex> lock(readyToUpdateChunksMutex);

		while(!readyToUpdateChunks.empty() && nbToUpdate-- != 0) {
			
			#ifdef TIME
				Profiler profiler("Graphic update (in loop)");
			#endif

			size_t currentChunkIndex = readyToUpdateChunks.front();
			readyToUpdateChunks.pop();

			allChunk_[currentChunkIndex]->graphicUpdate();
			allChunk_[currentChunkIndex]->state = 2;

			std::cout << "Chunk (x:" << allChunk_[currentChunkIndex]->getPosition().x << " y:" << allChunk_[currentChunkIndex]->getPosition().y << " z:" << allChunk_[currentChunkIndex]->getPosition().z << "): finished" << std::endl;

		}
	}

	std::tuple<int32_t, Chunk*, glm::uvec3> World::getVoxel(float x, float y, float z) const {

		float xSign = x < 0 ? -1 : 1;
		float ySign = y < 0 ? -1 : 1;
		float zSign = z < 0 ? -1 : 1;

		float trueChunkSize = chunkSize_ * voxelSize_;
		//float chunkLen = trueChunkSize - voxelSize_;

		int xIndex = std::floor(x / trueChunkSize);
		int yIndex = std::floor(y / trueChunkSize);
		int zIndex = std::floor(z / trueChunkSize);

		auto chunkIt = chunkCoordToIndex_.find(ChunkCoordinates(xIndex, yIndex, zIndex));
		if (chunkIt == chunkCoordToIndex_.end()) return {0, nullptr, glm::uvec3(0)}; //The chunk does not exist

		Chunk* chunkPtr = allChunk_[chunkIt->second].get();

		if (chunkPtr->state != 2) return {0, nullptr, glm::uvec3(0)};
		//TODO: We could return something else to make the user understand the chunk does not exist and make it jump directly to the next chunk

		//TODO: voir si ça marche avec des positions pas pile sur le cube
		int internalX = (x - xIndex * trueChunkSize) / voxelSize_;
		int internalY = (y - yIndex * trueChunkSize) / voxelSize_;
		int internalZ = (z - zIndex * trueChunkSize) / voxelSize_;

		return {chunkPtr->unsafeGetVoxel(internalX, internalY, internalZ), chunkPtr, glm::uvec3(internalX, internalY, internalZ)};

	}



	//TODO: tester les perf de cette fonction en utilisant plutôt des array et une boucle sur les 3 axes
	std::optional<CollisionFace> World::castRay(glm::vec3 const& startPosition, glm::vec3 const& direction, float maxDistance) const {
		
		#ifdef TIME
			Profiler profiler("Normal Ray");
		#endif

		glm::vec3 dir = glm::normalize(direction);

		//For processor following the IEC 60559 standard, adding 0.0f will get ride of the negative zero problem.
		// dir.x += 0.0f;
		// dir.y += 0.0f;
		// dir.z += 0.0f;

		//Positive or negative direction
		float xStep = std::signbit(dir.x) ? -1 : 1;
		float yStep = std::signbit(dir.y) ? -1 : 1;
		float zStep = std::signbit(dir.z) ? -1 : 1;

		//Index of the first voxel
		float x = floor(startPosition.x / voxelSize_) * voxelSize_;
		float y = floor(startPosition.y / voxelSize_) * voxelSize_;
		float z = floor(startPosition.z / voxelSize_) * voxelSize_;

		//Required time to exit the full voxel along each axis.
		float tMaxX = positiveModulo(startPosition.x, voxelSize_);
		float tMaxY = positiveModulo(startPosition.y, voxelSize_);
		float tMaxZ = positiveModulo(startPosition.z, voxelSize_);
		
		if (xStep == 1) tMaxX = voxelSize_ - tMaxX;
		if (yStep == 1) tMaxY = voxelSize_ - tMaxY;
		if (zStep == 1) tMaxZ = voxelSize_ - tMaxZ;

		tMaxX /= dir.x * xStep;
		tMaxY /= dir.y * yStep;
		tMaxZ /= dir.z * zStep;

		//Here we verify if our position are in between 2 positions. If so we position the ray casting just before the collision between the 2 positions
		if (std::fmod(startPosition.x, voxelSize_) == 0 && dir.x != 0.0){
			tMaxX = 0;
			//Note: we only need to shift the index if the direction is positive
			if (xStep == 1) x = x - xStep * voxelSize_;
		}
		if (std::fmod(startPosition.y, voxelSize_) == 0 && dir.y != 0.0){
			tMaxY = 0;
			if (yStep == 1) y = y - yStep * voxelSize_;
		}
		if (std::fmod(startPosition.z, voxelSize_) == 0 && dir.z != 0.0){
			tMaxZ = 0;
			if (zStep == 1) z = z - zStep * voxelSize_;
		}

		//Required time to do a full voxel length along the axis.
		float tDeltaX = voxelSize_ / dir.x * xStep;
		float tDeltaY = voxelSize_ / dir.y * yStep;
		float tDeltaZ = voxelSize_ / dir.z * zStep;

		//This value represent the distance traveled to get to the next voxel along the axis
		float travelingX = tMaxX;
		float travelingY = tMaxY;
		float travelingZ = tMaxZ;

		//TODO: Maybe we could remove this or maybe bot
		int movingSide = -1;
		int32_t foundVoxel = 0;
		Chunk* foundChunk = nullptr;
		glm::uvec3 localFoundPosition(0.0f);
		//const SimpleVoxel* foundVoxel = getVoxel(x + (voxelSize_ / 2.0f), y + (voxelSize_ / 2.0f), z + (voxelSize_ / 2.0f));		

		glm::vec3 traveledDistanceVector(0.0f);
		float traveledDistance = 0.0f;

		while(foundVoxel == 0) {

			//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5

			if(tMaxX < tMaxY) {

				if (tMaxX < tMaxZ) {

					tMaxX = tMaxX + tDeltaX;
					x = x + xStep * voxelSize_;
					movingSide = dir.x < 0 ? 2 : 4; 

					//Update traveled distance and now set traveling to a whole voxel
					traveledDistanceVector.x += dir.x * travelingX;
					travelingX = tDeltaX;

				}
				else {

					tMaxZ = tMaxZ + tDeltaZ;
					z = z + zStep * voxelSize_;
					movingSide = dir.z < 0 ? 1 : 3; 

					//Update traveled distance and now set traveling to a whole voxel
					traveledDistanceVector.z += dir.z * travelingZ;
					travelingZ = tDeltaZ;

				}

			}
			else {

				if (tMaxY < tMaxZ) {

					tMaxY = tMaxY + tDeltaY;
					y = y + yStep * voxelSize_;
					movingSide = dir.y < 0 ? 5 : 0; 

					//Update traveled distance and now set traveling to a whole voxel
					traveledDistanceVector.y += dir.y * travelingY;
					travelingY = tDeltaY;

				}
				else {
					
					tMaxZ = tMaxZ + tDeltaZ;
					z = z + zStep * voxelSize_;
					movingSide = dir.z < 0 ? 1 : 3; 

					//Update traveled distance and now set traveling to a whole voxel
					traveledDistanceVector.z += dir.z * travelingZ;
					travelingZ = tDeltaZ;
				}

			}

			traveledDistance = glm::length(traveledDistanceVector);

			if (traveledDistance > maxDistance) break;

			//We add half the voxel size to center it.
			std::tie(foundVoxel, foundChunk, localFoundPosition) = getVoxel(x + (voxelSize_ / 2.0f), y + (voxelSize_ / 2.0f), z + (voxelSize_ / 2.0f));

		}

		if (foundVoxel) {
			//TODO: remove this shit
			//if (movingSide == -1 ) return CollisionFace( foundVoxel->getFace(0), traveledDistance );

			return CollisionFace( foundVoxel, foundChunk, localFoundPosition, SimpleVoxel::getFaceNormal(movingSide), traveledDistance);

		}

		return {};

	}

	//TODO: Do with bug correction, better code, inspire from GLSL version
	std::optional<CollisionFace> World::castSVORay(glm::vec3 const& startPosition, glm::vec3 const& direction, float maxDistance) const {
		
		#ifdef TIME
			Profiler profiler("SVO Ray");
		#endif

		glm::vec3 position = startPosition;

		position /= voxelSize_;
		maxDistance /= voxelSize_;

		//Precompute for Ray Cast and for empty chunks...
		//Positive or negative direction
		glm::vec3 dir = glm::normalize(direction);
		dir += 0.0f;

		glm::vec3 directionSign = glm::step(0.0f, dir) * 2.0f - 1.0f;

		float totalTraveledDistance = 0.0f;

		auto [chunkIndex, localPosition] = getChunkLocalPosition(position, chunkSize_, 1.0f);

		//Save values for the castRay return
		int returnCode;
		float traveledDistance;
		std::optional<SVOCollisionInformation> collisionInformations;

		while (totalTraveledDistance <= maxDistance) {

			bool processable = false;

			Chunk* chunkPtr;
			auto chunkIt = chunkCoordToIndex_.find(ChunkCoordinates(chunkIndex.x, chunkIndex.y, chunkIndex.z));
			if (chunkIt != chunkCoordToIndex_.end()) {

				chunkPtr = allChunk_[chunkIt->second].get();

				if (chunkPtr->state == 2) processable = true;

			}

			//If impossible to process, travel like there is an empty leaf of size SVO len.
			if (!processable) {

				uint8_t minimumValueIndex;
				std::tie(traveledDistance, minimumValueIndex) = getTraveledDistanceAndIndex(localPosition, dir, directionSign, chunkSize_);

				glm::vec3 positionOffset = direction * traveledDistance;
				localPosition += positionOffset;

				if (totalTraveledDistance + traveledDistance > maxDistance) returnCode = 1;
				else returnCode = 2 + minimumValueIndex * 2 + (directionSign[minimumValueIndex] < 0);

			}
			else {

				std::tie(returnCode, traveledDistance, collisionInformations) = chunkPtr->svo_.castRay(localPosition, dir, maxDistance - totalTraveledDistance);

			}

			totalTraveledDistance += traveledDistance;

			//Found collision
			if (returnCode == 0) {

				int32_t foundVoxel = chunkPtr->unsafeGetVoxel(collisionInformations->position.x, collisionInformations->position.y, collisionInformations->position.z);

				return CollisionFace(foundVoxel, chunkPtr, collisionInformations->position, SimpleVoxel::getFaceNormal(collisionInformations->side), totalTraveledDistance * voxelSize_);

			}
			else if (returnCode == 1) {
				
				return {};

			}
			else {
				
				switch (returnCode) {

					case 2:
						localPosition.x = 0;
						++chunkIndex.x;
						break;

					case 3:
						localPosition.x = chunkSize_;
						--chunkIndex.x;
						break;

					case 4:
						localPosition.y = 0;
						++chunkIndex.y;
						break;

					case 5:
						localPosition.y = chunkSize_;
						--chunkIndex.y;
						break;

					case 6:
						localPosition.z = 0;
						++chunkIndex.z;
						break;

					case 7:
						localPosition.z = chunkSize_;
						--chunkIndex.z;
						break;

					default:
						std::cout << "Big error unhandled return code" << std::endl;
						return {};

    			}

			}

		}

		return {}; //totalTraveledDistance > maxDistance

	}


	std::pair<glm::vec3, glm::vec3> World::collide(mav::AABB const& box, glm::vec3 direction) const {

		#ifdef TIME
			Profiler profiler("World collide");
		#endif

		//TODO: Rendre ça paramétrable
		const float minimumDistanceToWall = 0.00001f;

		//Here we calculate the center and the extremities of our bounding box
		glm::vec3 boxCenterPosition = box.center;
		std::vector<glm::vec3> allExtremities {
			//Bottom face
			{-box.extents.x, -box.extents.y, -box.extents.z},
			{+box.extents.x, -box.extents.y, -box.extents.z},
			{+box.extents.x, -box.extents.y, +box.extents.z},
			{-box.extents.x, -box.extents.y, +box.extents.z},

			//Top face
			{-box.extents.x, +box.extents.y, -box.extents.z},
			{+box.extents.x, +box.extents.y, -box.extents.z},
			{+box.extents.x, +box.extents.y, +box.extents.z},
			{-box.extents.x, +box.extents.y, +box.extents.z},
		};
		
		glm::vec3 savedMovements(0.0f);
		glm::vec3 encounteredCollisions(0.0f);

		size_t it = 0;
		for (float directionLength = glm::length(direction); directionLength > 0; directionLength = glm::length(direction), ++it) {
			
			//Save the nearest collision
			int nearestIndex = -1;
			float nearestCollisionDistance = directionLength;
			float collisionDirection = 0.0f;

			for (glm::vec3 const& extremity : allExtremities) {

				std::optional<CollisionFace> foundFace = castRay(boxCenterPosition + extremity, direction, nearestCollisionDistance);

				if( foundFace ) {
					
					CollisionFace& face = *foundFace;

					//If this distance is higher than our nearest collision, ignore it.
					if (face.distance >= nearestCollisionDistance) continue;
					nearestCollisionDistance = face.distance;

					uint8_t axisIndex;
					for(axisIndex = 0; axisIndex < 3; ++axisIndex) {
						if (face.normal[axisIndex] != 0) break;
					}

					nearestIndex = axisIndex;
					collisionDirection = face.normal[axisIndex]; //This is the inverse direction

				}
			
			}

			//If nearestIndex is still equal to -1, it mean no collision was encountered, we can stop the algorithm
			if (nearestIndex == -1) break;

			//Movement done before collision
			glm::vec3 doneMovement = direction * (nearestCollisionDistance / directionLength);

			//We add a minimum distance to wall to prevent being in between 2 voxel positions
			doneMovement[nearestIndex] += minimumDistanceToWall * collisionDirection;
			encounteredCollisions[nearestIndex] = -collisionDirection;

			//Update center position
			boxCenterPosition += doneMovement;

			//Save the movement
			savedMovements += doneMovement;

			//Update direction
			direction -= doneMovement;
			direction[nearestIndex] = 0.0f;

		}

		//We return the total done movements and the remaining
		return {savedMovements + direction, encounteredCollisions};
		
	}

	size_t World::getChunkSize() const {
		return chunkSize_;
	}

	float World::getVoxelSize() const {
		return voxelSize_;
	}
	
}
