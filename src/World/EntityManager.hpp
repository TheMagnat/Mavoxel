
#pragma once

#include <Core/Global.hpp>
#include <VulkanWrapper/SSBO.hpp>
#include <Entity/Entity.hpp>
#include <Physics/PhysicSystem.hpp>

#include <glm/vec3.hpp>

#include <vector>

namespace mav {
    
    //TODO: peut être créer une classe mère en commun avec les SVO pour la partie GPU (getSSBO, updateBuffer...)
    class EntityManager {

        public:
            
            struct SimpleVoxel {
                alignas(16) glm::vec3 position;
            };


            EntityManager(const PhysicSystem* physicSystem) : physicSystem_(physicSystem), gpuBuffer_(Global::vulkanWrapper->generateSSBO()) {}

            Entity& operator[] (size_t index) {
                return entities_[index];
            }

            Entity& back() {
                return entities_.back();
            }

            void addEntity(AABB const& boundingBox) {
                entities_.emplace_back(boundingBox, 1.0f);
                entities_.back().setPhysicSystem(physicSystem_);
                updateBuffer();
            }

            void updateAll(float elapsedTime, World const& world) {
                for (Entity& entity : entities_) {
                    entity.update(elapsedTime, world);
                } 
            }


            //GPU part
            inline vuw::SSBO const& getSSBO() { return gpuBuffer_; }

            void updateBuffer() {
                
                std::vector<SimpleVoxel> positions;
                positions.reserve(entities_.size());

                for (Entity const& entity : entities_) {
                    positions.emplace_back(entity.getPosition());
                }

                gpuBuffer_.update<SimpleVoxel>(positions);
            }

        private:

            //Entities
            std::vector<Entity> entities_;
            
            //Saved physicSystem
            const PhysicSystem* physicSystem_;

            //GPU
            vuw::SSBO gpuBuffer_;

    };

}
