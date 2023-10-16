#pragma once

#include <World/World.hpp>
#include <Entity/Entity.hpp>

#include <glm/glm.hpp>

#include <Helper/Benchmark/Profiler.hpp>


namespace mav {

    class WorldLoader {

        // struct AroundChunks {
        //     std::vector<const vuw::SSBO*> octreeSSBOs;
        //     std::vector<int> octreeInformations;

        //     // A save of the chunks that are not initialized with their position and their index in the vector
        //     std::list<std::pair<glm::ivec3, size_t>> uninitializedChunks;

        //     // Clear all the data
        //     void clear() {
        //         octreeSSBOs.clear();
        //         octreeInformations.clear();
        //         uninitializedChunks.clear();
        //     }

        // };

        struct AroundChunks {

            struct Data {
                const vuw::SSBO* ssbo;
                int information;
            };

            //x, y, z
            std::list<std::list<std::list<Data>>> chunks;

            std::unordered_map<glm::ivec3, Data*> uninitializedChunks;


            inline void compute(World* world, Data& currentData, glm::ivec3 const chunkPosition) {

                const Chunk* chunkPtr = world->getChunk(chunkPosition);
                if (chunkPtr && chunkPtr->state == 2) {
                    currentData.ssbo = &chunkPtr->svo_.getSSBO();
                    currentData.information = 0;
                }
                else {
                    currentData.ssbo = nullptr;
                    currentData.information = 1;

                    uninitializedChunks.emplace(chunkPosition, &currentData);
                }

            }

            void update(World* world, glm::ivec3 const& centerPosition, glm::ivec3 const& positionOffset, int range) {
                
                #ifdef TIME
                    Profiler profiler("Update World Loader");
                #endif

                if (abs(positionOffset.x) > 1 || abs(positionOffset.y) > 1 || abs(positionOffset.z) > 1) {
                    //Here this computation is not worth it, recreate the whole data structure
                    fill(world, centerPosition, range);
                    return;
                }
                
                if (positionOffset.x > 0) {
                    
                    chunks.pop_front();
                    chunks.emplace_back();

                    std::list<std::list<Data>>& currentXList = chunks.back();
                    int x = range;

                    for (int y = -range; y <= range; ++y) {
                        currentXList.emplace_back();
                        std::list<Data>& currentYList = currentXList.back();

                        for (int z = -range; z <= range; ++z) {

                            currentYList.emplace_back();
                            Data& currentData = currentYList.back();

                            glm::ivec3 chunkPosition = centerPosition + glm::ivec3(x, y, z);

                            compute(world, currentData, chunkPosition);

                        }
                        

                    }

                }
                else if (positionOffset.x < 0) {

                    chunks.pop_back();
                    chunks.emplace_front();

                    std::list<std::list<Data>>& currentXList = chunks.front();
                    int x = -range;

                    for (int y = -range; y <= range; ++y) {
                        currentXList.emplace_back();
                        std::list<Data>& currentYList = currentXList.back();

                        for (int z = -range; z <= range; ++z) {

                            currentYList.emplace_back();
                            Data& currentData = currentYList.back();

                            glm::ivec3 chunkPosition = centerPosition + glm::ivec3(x, y, z);

                            compute(world, currentData, chunkPosition);

                        }
                        

                    }


                }

                if (positionOffset.y > 0) {

                    int x = -range;
                    for (auto& currentXList : chunks) {

                        currentXList.pop_front();
                        currentXList.emplace_back();

                        std::list<Data>& currentYList = currentXList.back();
                        int y = range;

                        for (int z = -range; z <= range; ++z) {

                            currentYList.emplace_back();
                            Data& currentData = currentYList.back();

                            glm::ivec3 chunkPosition = centerPosition + glm::ivec3(x, y, z);

                            compute(world, currentData, chunkPosition);

                        }
                        
                        ++x;
                    }

                }
                else if (positionOffset.y < 0) {

                    int x = -range;
                    for (auto& currentXList : chunks) {

                        currentXList.pop_back();
                        currentXList.emplace_front();

                        std::list<Data>& currentYList = currentXList.front();
                        int y = -range;

                        for (int z = -range; z <= range; ++z) {

                            currentYList.emplace_back();
                            Data& currentData = currentYList.back();

                            glm::ivec3 chunkPosition = centerPosition + glm::ivec3(x, y, z);

                            compute(world, currentData, chunkPosition);

                        }
                        
                        ++x;
                    }

                }

                if (positionOffset.z > 0) {

                    int x = -range;
                    for (auto& currentXList : chunks) {
                        
                        int y = -range;
                        for (auto& currentYList : currentXList) {

                            currentYList.pop_front();
                            currentYList.emplace_back();

                            Data& currentData = currentYList.back();
                            int z = range;

                            glm::ivec3 chunkPosition = centerPosition + glm::ivec3(x, y, z);

                            compute(world, currentData, chunkPosition);

                            ++y;
                        }

                        ++x;
                    }

                }
                else if (positionOffset.z < 0) {

                    int x = -range;
                    for (auto& currentXList : chunks) {

                        int y = -range;
                        for (auto& currentYList : currentXList) {

                            currentYList.pop_back();
                            currentYList.emplace_front();

                            Data& currentData = currentYList.front();
                            int z = -range;

                            glm::ivec3 chunkPosition = centerPosition + glm::ivec3(x, y, z);

                            compute(world, currentData, chunkPosition);

                            ++y;
                        }

                    ++x;
                    }

                }

            }

            void fill(World* world, glm::ivec3 const& centerPosition, int range) {
                
                #ifdef TIME
                    Profiler profiler("Fill World Loader");
                #endif

                chunks.clear();

                for (int x = -range; x <= range; ++x) {
                    chunks.emplace_back();
                    std::list<std::list<Data>>& currentXList = chunks.back();

                for (int y = -range; y <= range; ++y) {
                    currentXList.emplace_back();
                    std::list<Data>& currentYList = currentXList.back();

                for (int z = -range; z <= range; ++z) {
                    
                    currentYList.emplace_back();
                    Data& currentData = currentYList.back();

                    glm::ivec3 chunkPosition = centerPosition + glm::ivec3(x, y, z);

                    const Chunk* chunkPtr = world->getChunk(chunkPosition);
                    if (chunkPtr && chunkPtr->state == 2) {
                        currentData.ssbo = &chunkPtr->svo_.getSSBO();
                        currentData.information = 0;
                        // ssboDebug.push_back(&chunkPtr->svo_);
                        // std::cout << "1 -- Chunk initialized at " << chunkPosition.x << " " << chunkPosition.y << " " << chunkPosition.z << std::endl;
                    }
                    else {
                        currentData.ssbo = nullptr;
                        currentData.information = 1;

                        uninitializedChunks.emplace(chunkPosition, &currentData);
                        // ssboDebug.push_back(nullptr);
                        // std::cout << "Chunk uninitialized at " << chunkPosition.x << " " << chunkPosition.y << " " << chunkPosition.z << std::endl;
                    }
                    
                }
                }
                }


            }

        };

        public:
            WorldLoader(World* world, const Entity* focusedEntity, int range)
                : world_(world), focusedEntity_(focusedEntity), range_(range), size_(range * 2 + 1) {
                    initializeChunks();
                }


            
            void updateChunks(glm::ivec3 const& positionOffset) {




            }

            void update(float deltaTime) {


                #ifdef TIME
                    Profiler profiler("Update BASE World Loader");
                #endif

                glm::ivec3 newChunkPosition = world_->getChunkIndex(focusedEntity_->getPosition());

                if (newChunkPosition != lastChunkPosition_) {

                    //TODO: Mettre à jour le vector de chunks en fonction de la nouvelle position (faire une translation des chunks en fonction de l'offset ?)
                    glm::ivec3 positionOffset = newChunkPosition - lastChunkPosition_;

                    aroundChunks_.update(world_, newChunkPosition, positionOffset, range_);

                    //TODO: Mettre à jour les informations des chunks autour de la nouvelle position et pas juste réinitialiser à chaque fois
                    // initializeChunks();

                    lastChunkPosition_ = newChunkPosition;

                }
                else {

                    //If the focused entity is in the same chunk, we verify if an uninitialized chunk is now initialized
                    for (auto it = aroundChunks_.uninitializedChunks.begin(); it != aroundChunks_.uninitializedChunks.end();) {
                        
                        if ( glm::any( glm::greaterThan(glm::abs(it->first - newChunkPosition), glm::ivec3(range_)) ) ) {
                            it = aroundChunks_.uninitializedChunks.erase(it);
                            continue;
                        }


                        const Chunk* chunkPtr = world_->getChunk(it->first);

                        if (chunkPtr && chunkPtr->state == 2) {
                            it->second->ssbo = &chunkPtr->svo_.getSSBO();
                            it->second->information = 0;
                            it = aroundChunks_.uninitializedChunks.erase(it);
                            continue;
                        }
                        
                        ++it;

                    }

                }

            }

            void initializeChunks() {

                //Save the center position
                glm::ivec3 centerPosition = world_->getChunkIndex(focusedEntity_->getPosition());
                
                aroundChunks_.fill(world_, centerPosition, range_);
 
                //Save the current center chunk position for updates
                lastChunkPosition_ = centerPosition;

            }

            AroundChunks const& getAroundChunks() const { return aroundChunks_; }

            glm::ivec3 const& getCenterPosition() const { return lastChunkPosition_; }


            std::vector<const vuw::SSBO*> getSSBOs() const {

                #ifdef TIME
                    Profiler profiler("Get SSBOs");
                #endif


                std::vector<const vuw::SSBO*> ret;

                ret.reserve(size_ * size_ * size_);

                for (auto& currentXList : aroundChunks_.chunks) {
                    for (auto& currentYList : currentXList) {
                        for (auto& currentData : currentYList) {
                            ret.push_back(currentData.ssbo);
                        }
                    }
                }

                return ret;

            }

            std::vector<int> getInformations() const {
                #ifdef TIME
                    Profiler profiler("Get Informations");
                #endif
                std::vector<int> ret;

                ret.reserve(size_ * size_ * size_);

                for (auto& currentXList : aroundChunks_.chunks) {
                    for (auto& currentYList : currentXList) {
                        for (auto& currentData : currentYList) {
                            ret.push_back(currentData.information);
                        }
                    }
                }

                return ret;

            }


        private:
            World* world_;
            const Entity* focusedEntity_;

            // Range of chunks to load around the focused entity
            int range_;
            size_t size_;

            glm::ivec3 lastChunkPosition_;

            // Around chunks octree SSBOs and informations
            AroundChunks aroundChunks_;

    };

}
