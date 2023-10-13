#pragma once

#include <World/World.hpp>
#include <Entity/Entity.hpp>

#include <glm/vec3.hpp>


namespace mav {

    class WorldLoader {

        public:
            WorldLoader(const World* world, const Entity* focusedEntity)
                : world_(world), focusedEntity_(focusedEntity) {}


            void update(float deltaTime) {

                glm::ivec3 newChunkPosition = world_->getChunkIndex(focusedEntity_->getPosition());

                if (newChunkPosition != lastChunkPosition_) {
                    
                    //TODO: Mettre à jour le vector de chunks en fonction de la nouvelle position (faire une translation des chunks en fonction de l'offset ?)
                    
                    glm::ivec3 positionOffet = newChunkPosition - lastChunkPosition_;

                    lastChunkPosition_ = newChunkPosition;

                }
                
            }


        private:
            const World* world_;
            const Entity* focusedEntity_;

            glm::ivec3 lastChunkPosition_;

            // Saved octree SVO
            std::vector<const vuw::SSBO*> octreeSSBOs_;

    };

}
