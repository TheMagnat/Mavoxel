#pragma once

#include <World/World.hpp>
#include <Entity/Entity.hpp>

#include <glm/vec3.hpp>


namespace mav {

    class WorldLoader {

        struct AroundChunks {
            std::vector<const vuw::SSBO*> octreeSSBOs;
            std::vector<int> octreeInformations;

            // A save of the chunks that are not initialized with their position and their index in the vector
            std::list<std::pair<glm::ivec3, size_t>> uninitializedChunks;

            // Clear all the data
            void clear() {
                octreeSSBOs.clear();
                octreeInformations.clear();
                uninitializedChunks.clear();
            }

        };

        public:
            WorldLoader(const World* world, const Entity* focusedEntity, int range)
                : world_(world), focusedEntity_(focusedEntity), range_(range), size_(range * 2 + 1) {
                    initializeChunks();
                }


            void update(float deltaTime) {

                glm::ivec3 newChunkPosition = world_->getChunkIndex(focusedEntity_->getPosition());

                if (newChunkPosition != lastChunkPosition_) {

                    //TODO: Mettre à jour le vector de chunks en fonction de la nouvelle position (faire une translation des chunks en fonction de l'offset ?)
                    glm::ivec3 positionOffset = newChunkPosition - lastChunkPosition_;

                    // updateChunks(positionOffset);

                    //TODO: Mettre à jour les informations des chunks autour de la nouvelle position et pas juste réinitialiser à chaque fois
                    initializeChunks();

                    lastChunkPosition_ = newChunkPosition;

                }
                else {

                    //If the focused entity is in the same chunk, we verify if an uninitialized chunk is now initialized
                    for (auto it = aroundChunks_.uninitializedChunks.begin(); it != aroundChunks_.uninitializedChunks.end();) {
                        
                        const Chunk* chunkPtr = world_->getChunk(it->first);

                        if (chunkPtr && chunkPtr->state == 2) {
                            aroundChunks_.octreeSSBOs[it->second] = &chunkPtr->svo_.getSSBO();
                            aroundChunks_.octreeInformations[it->second] = 0;
                            std::cout << "Chunk initialized at " << it->first.x << " " << it->first.y << " " << it->first.z << std::endl;
                            it = aroundChunks_.uninitializedChunks.erase(it);
                            continue;
                        }
                        
                        ++it;

                    }

                }

            }

            void initializeChunks() {

                aroundChunks_.clear();
                aroundChunks_.octreeSSBOs.reserve(size_);
                aroundChunks_.octreeInformations.reserve(size_);

                //Save the center position
                glm::ivec3 centerPosition = world_->getChunkIndex(focusedEntity_->getPosition());

                for (int x = -range_; x <= range_; ++x) {
                for (int y = -range_; y <= range_; ++y) {
                for (int z = -range_; z <= range_; ++z) {
                    
                    glm::ivec3 chunkPosition = centerPosition + glm::ivec3(x, y, z);

                    const Chunk* chunkPtr = world_->getChunk(chunkPosition);
                    if (chunkPtr && chunkPtr->state == 2) {
                        aroundChunks_.octreeSSBOs.push_back(&chunkPtr->svo_.getSSBO());
                        aroundChunks_.octreeInformations.push_back(0);
                        // ssboDebug.push_back(&chunkPtr->svo_);
                        // std::cout << "1 -- Chunk initialized at " << chunkPosition.x << " " << chunkPosition.y << " " << chunkPosition.z << std::endl;
                    }
                    else {
                        aroundChunks_.octreeSSBOs.push_back(nullptr);
                        aroundChunks_.octreeInformations.push_back(1);
                        aroundChunks_.uninitializedChunks.emplace_back(chunkPosition, aroundChunks_.octreeSSBOs.size() - 1);
                        // ssboDebug.push_back(nullptr);
                        // std::cout << "Chunk uninitialized at " << chunkPosition.x << " " << chunkPosition.y << " " << chunkPosition.z << std::endl;
                    }
                    
                }
                }
                }
                
                //Save the current center chunk position for updates
                lastChunkPosition_ = centerPosition;

            }

            AroundChunks const& getAroundChunks() const { return aroundChunks_; }

            glm::ivec3 const& getCenterPosition() const { return lastChunkPosition_; }



        private:
            const World* world_;
            const Entity* focusedEntity_;

            // Range of chunks to load around the focused entity
            int range_;
            size_t size_;

            glm::ivec3 lastChunkPosition_;

            // Around chunks octree SSBOs and informations
            AroundChunks aroundChunks_;

    };

}
