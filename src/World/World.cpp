
#include <World/World.hpp>
#include <World/WorldPositionHelper.hpp>
#include <Core/Global.hpp>

#include <files/FileHandler.hpp>

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

	World::World(const VoxelMapGenerator * voxelMapGenerator, size_t octreeDepth, float voxelSize, std::string const& worldFolderPath, bool newWorld)
		//TODO: Rendre le nombre de thread paramétrable
		: voxelMapGenerator_(voxelMapGenerator), octreeDepth_(octreeDepth), chunkSize_(std::pow(2, octreeDepth)), voxelSize_(voxelSize), threadPool(8),
		chunkIndexer_(worldFolderPath), fileSaving_(!worldFolderPath.empty()), worldFolderPath_(worldFolderPath)
		{

			if (!newWorld) {

				if (!fileSaving_) throw std::invalid_argument("Impossible to load an old world if empty folder path given.");

				chunkIndexer_.load();

			}

		}

	void World::loadOrCreateChunk(glm::ivec3 const& chunkPosition) {

		size_t index = chunkIndexer_.getChunk(chunkPosition);
		
		if (index == 0) {
		
			createChunk(chunkPosition);
		
		}
		else {

			loadChunk(chunkPosition, index);

		}

	}

	void World::createChunk(glm::ivec3 const& chunkPosition){

		// size_t newChunkIndex = allChunk_.size();

		// allChunk_.push_back( std::make_unique<Chunk>(this, chunkPosition, octreeDepth_, voxelSize_) );
		auto emplaceResult = chunks_.try_emplace(chunkPosition, this, chunkPosition, octreeDepth_, voxelSize_);
		if (!emplaceResult.second) {
			//TODO: create std::cout for glm::ivec3, glm::vec3...
			std::cout << "Impossible to create Chunk at position X X X." << std::endl;
			return;
		}

		Chunk* currentChunkPtr = &(emplaceResult.first->second);

		//TODO: faire un truc du rez ou changer la classe threadPool pour ne plus rien renvoyer
		auto rez = threadPool.enqueue([this, currentChunkPtr](){
			
			std::cout << "Start world generation of " << currentChunkPtr->getPosition().x << " " << currentChunkPtr->getPosition().y << " " << currentChunkPtr->getPosition().z << std::endl;

			#ifdef TIME
				Profiler profiler("Full chunks generation");
			#endif

			currentChunkPtr->generateVoxels(voxelMapGenerator_);
			currentChunkPtr->state = 1;

			chunkIndexer_.addChunk(currentChunkPtr->getPosition());

			// Now thread safe operation
			std::lock_guard<std::mutex> lock(readyToUpdateChunksMutex);
			readyToUpdateChunks.push(currentChunkPtr);

			std::cout << "End world generation of " << currentChunkPtr->getPosition().x << " " << currentChunkPtr->getPosition().y << " " << currentChunkPtr->getPosition().z << std::endl;

		});

	}


	void World::loadChunk(glm::ivec3 const& chunkPosition, size_t fileIndex) {

		// size_t newChunkIndex = allChunk_.size();

		auto emplaceResult = chunks_.try_emplace(chunkPosition, this, chunkPosition, octreeDepth_, voxelSize_);
		if (!emplaceResult.second) {
			//TODO: create std::cout for glm::ivec3, glm::vec3...
			std::cout << "Impossible to create Chunk at position X X X." << std::endl;
			return;
		}

		Chunk* currentChunkPtr = &(emplaceResult.first->second);

		//TODO: faire un truc du rez ou changer la classe threadPool pour ne plus rien renvoyer
		auto rez = threadPool.enqueue([this, currentChunkPtr, fileIndex](){
			
			#ifdef TIME
				Profiler profiler("Full chunks loading");
			#endif

			std::ifstream stream = mav::getReadFileStream(std::to_string(fileIndex) + ".mvx", worldFolderPath_);

			currentChunkPtr->loadVoxels(&stream);
			currentChunkPtr->state = 1;

			// Now thread safe operation
			std::lock_guard<std::mutex> lock(readyToUpdateChunksMutex);
			readyToUpdateChunks.push(currentChunkPtr);

		});


	}

	void World::save() {

		chunkIndexer_.save(chunks_);

	}

	void World::bulkCreateChunk(glm::vec3 position, float createDistance, bool sorted) {

		for (glm::vec3 const& chunkPosition : getAroundChunks(position, createDistance, sorted)) {

			//Try finding the chunk
			auto chunkIt = chunks_.find(chunkPosition);
			if (chunkIt != chunks_.end()) continue;
			
			//If it does not exist, we create it
			loadOrCreateChunk(chunkPosition);

		}

	}

	Chunk* World::getChunk(int x, int y, int z) {
		auto chunkIt = chunks_.find(glm::ivec3(x, y, z));
		if (chunkIt == chunks_.end()) return nullptr;
		
		Chunk* foundChunk = &chunkIt->second;
		if (foundChunk->state == 0) return nullptr;

		return foundChunk;
	}

	Chunk* World::getChunk(glm::ivec3 const& position) {
		auto chunkIt = chunks_.find(position);
		if (chunkIt == chunks_.end()) return nullptr;
		
		Chunk* foundChunk = &chunkIt->second;
		if (foundChunk->state == 0) return nullptr;

		return foundChunk;
	}

	Chunk* World::getChunkFromWorldPos(glm::vec3 position) {

		//Calculate center position		
		float trueChunkSize = chunkSize_ * voxelSize_;

		int xIndex = floor(position.x / trueChunkSize);
		int yIndex = floor(position.y / trueChunkSize);
		int zIndex = floor(position.z / trueChunkSize);

		auto chunkIt = chunks_.find(glm::ivec3(xIndex, yIndex, zIndex));
		if (chunkIt == chunks_.end()) return nullptr;
		
		Chunk* foundChunk = &chunkIt->second;
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

			Chunk* currentChunk = readyToUpdateChunks.front();
			readyToUpdateChunks.pop();

			currentChunk->graphicUpdate();
			currentChunk->state = 2;

			// std::cout << "Chunk (x:" << currentChunk->getPosition().x << " y:" << currentChunk->getPosition().y << " z:" << currentChunk->getPosition().z << "): finished" << std::endl;

		}
	}

	std::tuple<int32_t, const Chunk*, glm::uvec3> World::getVoxel(float x, float y, float z) const {

		float xSign = x < 0 ? -1 : 1;
		float ySign = y < 0 ? -1 : 1;
		float zSign = z < 0 ? -1 : 1;

		float trueChunkSize = chunkSize_ * voxelSize_;
		//float chunkLen = trueChunkSize - voxelSize_;

		int xIndex = std::floor(x / trueChunkSize);
		int yIndex = std::floor(y / trueChunkSize);
		int zIndex = std::floor(z / trueChunkSize);

		auto chunkIt = chunks_.find(glm::ivec3(xIndex, yIndex, zIndex));
		if (chunkIt == chunks_.end()) return {0, nullptr, glm::uvec3(0)}; //The chunk does not exist

		const Chunk* chunkPtr = &chunkIt->second;

		if (chunkPtr->state != 2) return {0, nullptr, glm::uvec3(0)};
		//TODO: We could return something else to make the user understand the chunk does not exist and make it jump directly to the next chunk

		//TODO: voir si ça marche avec des positions pas pile sur le cube
		int internalX = (x - xIndex * trueChunkSize) / voxelSize_;
		int internalY = (y - yIndex * trueChunkSize) / voxelSize_;
		int internalZ = (z - zIndex * trueChunkSize) / voxelSize_;

		return {chunkPtr->unsafeGetVoxel(internalX, internalY, internalZ), chunkPtr, glm::uvec3(internalX, internalY, internalZ)};

	}



	//TODO: tester les perf de cette fonction en utilisant plutôt des array et une boucle sur les 3 axes
	// std::optional<RayCollisionInformations> World::castRay(glm::vec3 const& startPosition, glm::vec3 const& direction, float maxDistance) const {
		
	// 	#ifdef TIME
	// 		Profiler profiler("Normal Ray");
	// 	#endif

	// 	glm::vec3 dir = glm::normalize(direction);

	// 	//For processor following the IEC 60559 standard, adding 0.0f will get ride of the negative zero problem.
	// 	// dir.x += 0.0f;
	// 	// dir.y += 0.0f;
	// 	// dir.z += 0.0f;

	// 	//Positive or negative direction
	// 	float xStep = std::signbit(dir.x) ? -1 : 1;
	// 	float yStep = std::signbit(dir.y) ? -1 : 1;
	// 	float zStep = std::signbit(dir.z) ? -1 : 1;

	// 	//Index of the first voxel
	// 	float x = floor(startPosition.x / voxelSize_) * voxelSize_;
	// 	float y = floor(startPosition.y / voxelSize_) * voxelSize_;
	// 	float z = floor(startPosition.z / voxelSize_) * voxelSize_;

	// 	//Required time to exit the full voxel along each axis.
	// 	float tMaxX = positiveModulo(startPosition.x, voxelSize_);
	// 	float tMaxY = positiveModulo(startPosition.y, voxelSize_);
	// 	float tMaxZ = positiveModulo(startPosition.z, voxelSize_);
		
	// 	if (xStep == 1) tMaxX = voxelSize_ - tMaxX;
	// 	if (yStep == 1) tMaxY = voxelSize_ - tMaxY;
	// 	if (zStep == 1) tMaxZ = voxelSize_ - tMaxZ;

	// 	tMaxX /= dir.x * xStep;
	// 	tMaxY /= dir.y * yStep;
	// 	tMaxZ /= dir.z * zStep;

	// 	//Here we verify if our position are in between 2 positions. If so we position the ray casting just before the collision between the 2 positions
	// 	if (std::fmod(startPosition.x, voxelSize_) == 0 && dir.x != 0.0){
	// 		tMaxX = 0;
	// 		//Note: we only need to shift the index if the direction is positive
	// 		if (xStep == 1) x = x - xStep * voxelSize_;
	// 	}
	// 	if (std::fmod(startPosition.y, voxelSize_) == 0 && dir.y != 0.0){
	// 		tMaxY = 0;
	// 		if (yStep == 1) y = y - yStep * voxelSize_;
	// 	}
	// 	if (std::fmod(startPosition.z, voxelSize_) == 0 && dir.z != 0.0){
	// 		tMaxZ = 0;
	// 		if (zStep == 1) z = z - zStep * voxelSize_;
	// 	}

	// 	//Required time to do a full voxel length along the axis.
	// 	float tDeltaX = voxelSize_ / dir.x * xStep;
	// 	float tDeltaY = voxelSize_ / dir.y * yStep;
	// 	float tDeltaZ = voxelSize_ / dir.z * zStep;

	// 	//This value represent the distance traveled to get to the next voxel along the axis
	// 	float travelingX = tMaxX;
	// 	float travelingY = tMaxY;
	// 	float travelingZ = tMaxZ;

	// 	//TODO: Maybe we could remove this or maybe bot
	// 	int movingSide = -1;
	// 	int32_t foundVoxel = 0;
	// 	Chunk* foundChunk = nullptr;
	// 	glm::uvec3 localFoundPosition(0.0f);
	// 	//const SimpleVoxel* foundVoxel = getVoxel(x + (voxelSize_ / 2.0f), y + (voxelSize_ / 2.0f), z + (voxelSize_ / 2.0f));		

	// 	glm::vec3 traveledDistanceVector(0.0f);
	// 	float traveledDistance = 0.0f;

	// 	while(foundVoxel == 0) {

	// 		//bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5

	// 		if(tMaxX < tMaxY) {

	// 			if (tMaxX < tMaxZ) {

	// 				tMaxX = tMaxX + tDeltaX;
	// 				x = x + xStep * voxelSize_;
	// 				movingSide = dir.x < 0 ? 2 : 4; 

	// 				//Update traveled distance and now set traveling to a whole voxel
	// 				traveledDistanceVector.x += dir.x * travelingX;
	// 				travelingX = tDeltaX;

	// 			}
	// 			else {

	// 				tMaxZ = tMaxZ + tDeltaZ;
	// 				z = z + zStep * voxelSize_;
	// 				movingSide = dir.z < 0 ? 1 : 3; 

	// 				//Update traveled distance and now set traveling to a whole voxel
	// 				traveledDistanceVector.z += dir.z * travelingZ;
	// 				travelingZ = tDeltaZ;

	// 			}

	// 		}
	// 		else {

	// 			if (tMaxY < tMaxZ) {

	// 				tMaxY = tMaxY + tDeltaY;
	// 				y = y + yStep * voxelSize_;
	// 				movingSide = dir.y < 0 ? 5 : 0; 

	// 				//Update traveled distance and now set traveling to a whole voxel
	// 				traveledDistanceVector.y += dir.y * travelingY;
	// 				travelingY = tDeltaY;

	// 			}
	// 			else {
					
	// 				tMaxZ = tMaxZ + tDeltaZ;
	// 				z = z + zStep * voxelSize_;
	// 				movingSide = dir.z < 0 ? 1 : 3; 

	// 				//Update traveled distance and now set traveling to a whole voxel
	// 				traveledDistanceVector.z += dir.z * travelingZ;
	// 				travelingZ = tDeltaZ;
	// 			}

	// 		}

	// 		traveledDistance = glm::length(traveledDistanceVector);

	// 		if (traveledDistance > maxDistance) break;

	// 		//We add half the voxel size to center it.
	// 		std::tie(foundVoxel, foundChunk, localFoundPosition) = getVoxel(x + (voxelSize_ / 2.0f), y + (voxelSize_ / 2.0f), z + (voxelSize_ / 2.0f));

	// 	}

	// 	if (foundVoxel) {
	// 		//TODO: remove this shit
	// 		//if (movingSide == -1 ) return CollisionFace( foundVoxel->getFace(0), traveledDistance );

	// 		return CollisionFace( foundVoxel, foundChunk, localFoundPosition, std::vector<glm::vec3>{SimpleVoxel::getFaceNormal(movingSide)}, traveledDistance);

	// 	}

	// 	return {};

	// }


	std::optional<RayCollisionInformations> World::castRay(glm::vec3 position, glm::vec3 const& inputDirection, float maxDistance) {
		
		static const std::vector<glm::ivec2> codeToOffset { glm::ivec2(0, 1), glm::ivec2(0, -1), glm::ivec2(1, 1), glm::ivec2(1, -1), glm::ivec2(2, 1), glm::ivec2(2, -1) };

		glm::vec3 direction = glm::normalize(inputDirection);

		position /= voxelSize_;
		maxDistance /= voxelSize_;

		glm::vec3 directionSign = glm::step(0.0f, direction) * 2.0f - 1.0f;

		auto [chunkPosition, localPosition] = getChunkLocalPosition(position, chunkSize_);
		roundPosition(localPosition);
		
		//To ensure that we do not start on the limit of a chunk tower the direction vector
		shiftOnLimitChunk(chunkPosition, localPosition, directionSign, chunkSize_);

		//Store result between iterations
		int returnCode;
		float traveledDistance;

		glm::vec3 hitPosition;
		Chunk* chunkPtr = nullptr;

		std::optional<SVOCollisionInformation> collisionInformations;
		
		//Store normal between iterations in case of first voxel hitted to keep the correct normal
		glm::vec3 normal = glm::vec3(0.0);
		std::vector<glm::vec3> normals;

		int count = 0;
		float totalTraveledDistance = 0.0f;
		while (totalTraveledDistance < maxDistance) {

			if (count++ > 100) {
                std::cout << "Infinite World Cast ray" << std::endl;
            }
			
			bool processable = false;
			auto chunkIt = chunks_.find(chunkPosition);
			if (chunkIt != chunks_.end()) {

				chunkPtr = &chunkIt->second;

				if (chunkPtr->state == 2) processable = true;

			}
			else {
				chunkPtr = nullptr;
			}

			//If impossible to process, travel like there is an empty leaf of size SVO len.
			if (!processable) {
				
				uint8_t minimumValueIndex;
				std::tie(traveledDistance, minimumValueIndex) = getShortestTraveledDistanceAndIndex(localPosition, direction, directionSign, chunkSize_);

				glm::vec3 positionOffset = direction * traveledDistance;
				
				localPosition += positionOffset;
				roundPosition(localPosition);

				if (totalTraveledDistance + traveledDistance > maxDistance) returnCode = 1;
				else returnCode = 2 + minimumValueIndex * 2 + int(directionSign[minimumValueIndex] < 0);

				//Compute normals
				glm::vec3 crossedSides = glm::equal(glm::fract(glm::mod(localPosition, (float)chunkSize_)), glm::vec3(0.0f));
				normals.clear();
				for (uint8_t i = 0; i < 3; ++i) {
					if (crossedSides[i] != 0) {
						normals.emplace_back(0.0f);
						normals.back()[i] = -directionSign[i];
					}
				}

			}
			else {
				std::tie(returnCode, traveledDistance, collisionInformations) = chunkPtr->svo_.castRay(localPosition, direction, maxDistance - totalTraveledDistance, normals);
			}

			totalTraveledDistance += traveledDistance;

			if (returnCode == 0) {
				// glm::vec3 worldPosition = localPosition + glm::vec3(chunkPosition * (int)chunkSize_);
				return RayCollisionInformations(collisionInformations->id, collisionInformations->position, normals, totalTraveledDistance * voxelSize_, chunkPtr);
			}
			else if (returnCode == 1) {
				return {};
			}
			else {
				
					//Note: We could call shiftOnLimitChunk here instead of using the return
					//TODO: Maybe the problem of ANGLE is here
					glm::ivec2 newChunkOffset = codeToOffset[returnCode - 2];
					localPosition[newChunkOffset[0]] = ( (-newChunkOffset[1] + 1) / 2 ) * float(chunkSize_); //This calcul will transform 1 to 0 and -1 to 1
					chunkPosition[newChunkOffset[0]] += newChunkOffset[1];
				
			}
			
		}

		return {};

	}

	
	std::optional<RayCollisionInformations> World::castRay(glm::vec3 position, glm::vec3 const& inputDirection, float maxDistance) const {
		
		static const std::vector<glm::ivec2> codeToOffset { glm::ivec2(0, 1), glm::ivec2(0, -1), glm::ivec2(1, 1), glm::ivec2(1, -1), glm::ivec2(2, 1), glm::ivec2(2, -1) };

		glm::vec3 direction = glm::normalize(inputDirection);

		position /= voxelSize_;
		maxDistance /= voxelSize_;

		glm::vec3 directionSign = glm::step(0.0f, direction) * 2.0f - 1.0f;

		auto [chunkPosition, localPosition] = getChunkLocalPosition(position, chunkSize_);
		roundPosition(localPosition);
		
		//To ensure that we do not start on the limit of a chunk tower the direction vector
		shiftOnLimitChunk(chunkPosition, localPosition, directionSign, chunkSize_);

		//Store result between iterations
		int returnCode;
		float traveledDistance;

		glm::vec3 hitPosition;
		const Chunk* chunkPtr = nullptr;

		std::optional<SVOCollisionInformation> collisionInformations;
		
		//Store normal between iterations in case of first voxel hitted to keep the correct normal
		glm::vec3 normal = glm::vec3(0.0);
		std::vector<glm::vec3> normals;

		int count = 0;
		float totalTraveledDistance = 0.0f;
		while (totalTraveledDistance < maxDistance) {

			if (count++ > 100) {
                std::cout << "Infinite World Cast ray" << std::endl;
            }
			
			bool processable = false;
			auto chunkIt = chunks_.find(chunkPosition);
			if (chunkIt != chunks_.end()) {

				chunkPtr = &chunkIt->second;

				if (chunkPtr->state == 2) processable = true;

			}
			else {
				chunkPtr = nullptr;
			}

			//If impossible to process, travel like there is an empty leaf of size SVO len.
			if (!processable) {
				
				uint8_t minimumValueIndex;
				std::tie(traveledDistance, minimumValueIndex) = getShortestTraveledDistanceAndIndex(localPosition, direction, directionSign, chunkSize_);

				glm::vec3 positionOffset = direction * traveledDistance;
				
				localPosition += positionOffset;
				roundPosition(localPosition);

				if (totalTraveledDistance + traveledDistance > maxDistance) returnCode = 1;
				else returnCode = 2 + minimumValueIndex * 2 + int(directionSign[minimumValueIndex] < 0);

				//Compute normals
				glm::vec3 crossedSides = glm::equal(glm::fract(glm::mod(localPosition, (float)chunkSize_)), glm::vec3(0.0f));
				normals.clear();
				for (uint8_t i = 0; i < 3; ++i) {
					if (crossedSides[i] != 0) {
						normals.emplace_back(0.0f);
						normals.back()[i] = -directionSign[i];
					}
				}

			}
			else {
				std::tie(returnCode, traveledDistance, collisionInformations) = chunkPtr->svo_.castRay(localPosition, direction, maxDistance - totalTraveledDistance, normals);
			}

			totalTraveledDistance += traveledDistance;

			if (returnCode == 0) {
				// glm::vec3 worldPosition = localPosition + glm::vec3(chunkPosition * (int)chunkSize_);
				return RayCollisionInformations(collisionInformations->id, collisionInformations->position, normals, totalTraveledDistance * voxelSize_);
			}
			else if (returnCode == 1) {
				return {};
			}
			else {
				
					//Note: We could call shiftOnLimitChunk here instead of using the return
					//TODO: Maybe the problem of ANGLE is here
					glm::ivec2 newChunkOffset = codeToOffset[returnCode - 2];
					localPosition[newChunkOffset[0]] = ( (-newChunkOffset[1] + 1) / 2 ) * float(chunkSize_); //This calcul will transform 1 to 0 and -1 to 1
					chunkPosition[newChunkOffset[0]] += newChunkOffset[1];
				
			}
			
		}

		return {};

	}

	//TODO: Do with bug correction, better code, inspire from GLSL version
	// std::optional<RayCollisionInformations> World::castSVORay(glm::vec3 const& startPosition, glm::vec3 const& direction, float maxDistance) const {
		
	// 	#ifdef TIME
	// 		Profiler profiler("SVO Ray");
	// 	#endif

	// 	glm::vec3 position = startPosition;

	// 	position /= voxelSize_;
	// 	maxDistance /= voxelSize_;

	// 	//Precompute for Ray Cast and for empty chunks...
	// 	//Positive or negative direction
	// 	glm::vec3 dir = glm::normalize(direction);
	// 	dir += 0.0f;

	// 	glm::vec3 directionSign = glm::step(0.0f, dir) * 2.0f - 1.0f;

	// 	float totalTraveledDistance = 0.0f;

	// 	auto [chunkIndex, localPosition] = getChunkLocalPosition(position, chunkSize_);

	// 	//Save values for the castRay return
	// 	int returnCode;
	// 	float traveledDistance;
	// 	std::optional<SVOCollisionInformation> collisionInformations;

	// 	while (totalTraveledDistance <= maxDistance) {

	// 		bool processable = false;

	// 		Chunk* chunkPtr;
	// 		auto chunkIt = chunks_.find(ChunkCoordinates(chunkIndex.x, chunkIndex.y, chunkIndex.z));
	// 		if (chunkIt != chunks_.end()) {

	// 			chunkPtr = allChunk_[chunkIt->second].get();

	// 			if (chunkPtr->state == 2) processable = true;

	// 		}

	// 		//If impossible to process, travel like there is an empty leaf of size SVO len.
	// 		if (!processable) {

	// 			uint8_t minimumValueIndex;
	// 			std::tie(traveledDistance, minimumValueIndex) = getShortestTraveledDistanceAndIndex(localPosition, dir, directionSign, chunkSize_);

	// 			glm::vec3 positionOffset = direction * traveledDistance;
	// 			localPosition += positionOffset;

	// 			if (totalTraveledDistance + traveledDistance > maxDistance) returnCode = 1;
	// 			else returnCode = 2 + minimumValueIndex * 2 + (directionSign[minimumValueIndex] < 0);

	// 		}
	// 		else {

	// 			std::tie(returnCode, traveledDistance, collisionInformations) = chunkPtr->svo_.castRay(localPosition, dir, maxDistance - totalTraveledDistance);

	// 		}

	// 		totalTraveledDistance += traveledDistance;

	// 		//Found collision
	// 		if (returnCode == 0) {

	// 			int32_t foundVoxel = chunkPtr->unsafeGetVoxel(collisionInformations->position.x, collisionInformations->position.y, collisionInformations->position.z);

	// 			return CollisionFace(foundVoxel, chunkPtr, collisionInformations->position, SimpleVoxel::getFaceNormal(collisionInformations->side), totalTraveledDistance * voxelSize_);

	// 		}
	// 		else if (returnCode == 1) {
				
	// 			return {};

	// 		}
	// 		else {
				
	// 			switch (returnCode) {

	// 				case 2:
	// 					localPosition.x = 0;
	// 					++chunkIndex.x;
	// 					break;

	// 				case 3:
	// 					localPosition.x = chunkSize_;
	// 					--chunkIndex.x;
	// 					break;

	// 				case 4:
	// 					localPosition.y = 0;
	// 					++chunkIndex.y;
	// 					break;

	// 				case 5:
	// 					localPosition.y = chunkSize_;
	// 					--chunkIndex.y;
	// 					break;

	// 				case 6:
	// 					localPosition.z = 0;
	// 					++chunkIndex.z;
	// 					break;

	// 				case 7:
	// 					localPosition.z = chunkSize_;
	// 					--chunkIndex.z;
	// 					break;

	// 				default:
	// 					std::cout << "Big error unhandled return code" << std::endl;
	// 					return {};

    // 			}

	// 		}

	// 	}

	// 	return {}; //totalTraveledDistance > maxDistance

	// }


	std::pair<glm::vec3, glm::vec3> World::collide(mav::AABB const& box, glm::vec3 direction) const {

		#ifdef TIME
			Profiler profiler("World collide");
		#endif

		glm::vec3 originalDirectionSign = glm::step(0.0f, direction) * 2.0f - 1.0f;

		//TODO: Rendre ça paramétrable
		const float minimumDistanceToWall = 0.0001f;

		//Here we calculate the center and the extremities of our bounding box
		glm::vec3 boxCenterPosition = box.center;
		glm::vec3 originalBoxCenterPosition = box.center;
		std::vector<glm::vec3> allExtremities;
		

		//Here we prepare the computation of all extremities by counting the number of point we need to compute on each axis to not skip any voxel size.
		glm::vec3 axisPointsNb = glm::ivec3(box.extents * 2 / voxelSize_) + 1; //The +1 here force the first point to be counted, and the <= in the loops test force the last point to be counted too.
		glm::vec3 axisIndex;
		allExtremities.reserve( (axisPointsNb.x + 1) * (axisPointsNb.y + 1) * (axisPointsNb.z + 1) );

		for (axisIndex.x = 0; axisIndex.x <= axisPointsNb.x; ++axisIndex.x) {
			for (axisIndex.y = 0; axisIndex.y <= axisPointsNb.y; ++axisIndex.y) {
				for (axisIndex.z = 0; axisIndex.z <= axisPointsNb.z; ++axisIndex.z) {

					allExtremities.emplace_back(-box.extents + (box.extents * 2) * (axisIndex / axisPointsNb));

				}
			}
		}

		// glm::vec3 boxCenterPosition = box.center;
		// std::vector<glm::vec3> allExtremities {
		// 	//Bottom face
		// 	{-box.extents.x, -box.extents.y, -box.extents.z},
		// 	{+box.extents.x, -box.extents.y, -box.extents.z},
		// 	{+box.extents.x, -box.extents.y, +box.extents.z},
		// 	{-box.extents.x, -box.extents.y, +box.extents.z},

		// 	//Top face
		// 	{-box.extents.x, +box.extents.y, -box.extents.z},
		// 	{+box.extents.x, +box.extents.y, -box.extents.z},
		// 	{+box.extents.x, +box.extents.y, +box.extents.z},
		// 	{-box.extents.x, +box.extents.y, +box.extents.z},
		// };
		
		glm::vec3 savedMovements(0.0f);
		glm::vec3 encounteredCollisions(0.0f);

		int count = 0;
		for (float directionLength = glm::length(direction); directionLength > 0; directionLength = glm::length(direction)) {
			
			if (count++ > 100) {
				std::cout << "lol count > 100" << std::endl;
			}

			//Save the nearest collision
			bool foundCollision = false;
			float nearestCollisionDistance = directionLength;
			glm::vec3 collisionDirection;

			for (glm::vec3 const& extremity : allExtremities) {
				
				std::optional<RayCollisionInformations> collisionInformations = castRay(boxCenterPosition + extremity, direction, nearestCollisionDistance);

				glm::vec3 tempoTest = boxCenterPosition + extremity;

				glm::vec3 tempoTest2 = originalBoxCenterPosition + extremity;
				
				glm::vec3 newPos = originalBoxCenterPosition + savedMovements;
				glm::vec3 tempoTest3 = newPos + extremity;



				std::optional<RayCollisionInformations> test2 = castRay(boxCenterPosition + extremity, direction, nearestCollisionDistance);

				if( collisionInformations ) {
					
					RayCollisionInformations& collision = *collisionInformations;					

					//If this distance is higher than our nearest collision, ignore it.
					if (collision.distance >= nearestCollisionDistance) continue;

					std::optional<RayCollisionInformations> bonusCollision = castRay(boxCenterPosition + extremity, direction, nearestCollisionDistance);

					foundCollision = true;
					nearestCollisionDistance = collision.distance;

					if (collision.normals.empty()) {
						std::cout << "Empty shit" << std::endl;
					}

					collisionDirection = glm::vec3(0.0f);
					for (glm::vec3 const& normal : collision.normals) {
						collisionDirection += normal;
					}
					
				}
			
			}

			//If nearestIndex is still equal to -1, it mean no collision was encountered, we can stop the algorithm
			if (!foundCollision) break;

			//Movement done before collision
			glm::vec3 doneMovement = direction * ((nearestCollisionDistance) / directionLength);

			//We add a minimum distance to wall to prevent being in between 2 voxel positions
			doneMovement += minimumDistanceToWall * collisionDirection;

			//Save the encountered collisions axis
			encounteredCollisions -= collisionDirection;

			//Update center position
			boxCenterPosition += doneMovement;

			//Save the movement
			savedMovements += doneMovement;

			//Update direction
			direction -= doneMovement;
			direction -= direction * glm::abs(collisionDirection); //Set to 0 in the encountered collisions axis

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
