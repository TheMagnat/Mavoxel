
#pragma once

#include <glm/vec3.hpp>

#include <Collision/AABB.hpp>
#include <Physics/PhysicSystem.hpp>
#include <World/World.hpp>

#include <Mesh/DebugVoxel.hpp>

namespace mav {


    class Entity {

        public:
        //TODO: peut être avoir un bool qui vérifie qu'on peut utiliser front et right ?
            Entity(AABB boundingBox, float massP);

            void setPhysicSystem(const PhysicSystem* newPhysicSystem);


            //Movement logic
            virtual void addVelocity(glm::vec3 const& direction, float strength, float deltaTime);
            void addVelocity(glm::vec3 const& direction, float strength, float deltaTime, glm::vec3 const& front, glm::vec3 const& right);

            virtual void setAcceleration(glm::vec3 const& forceDirection, float magnitude);
            void setAcceleration(glm::vec3 const& forceDirection, float magnitude, glm::vec3 const& front, glm::vec3 const& right);

            //Jump / Ground logic
            void jump(float strength);
            void groundHit();
            void groundLeft();
            
            //Update the position
            void update(float elapsedTime);
            bool update(float elapsedTime, World const& world);
            
            //Getters
            inline glm::vec3 const& getPosition() const { return boundingBox_.center; }
            inline AABB const& getBoundingBox() const { return boundingBox_; }

        public:

            //Movement / Velocity
            glm::vec3 velocity;
            glm::vec3 movement;
            glm::vec3 acceleration;

            //Entity informations
            float mass;

        protected:

            //Jump information
            bool onTheGround = false;
            
            int remainingJumps = 0;
            int maxJumps = 2;
            float timeSinceLastJump = 0.0f;
            float timeLimitBetweenJumps = 0.25f;
            
            //Collision
            AABB boundingBox_;

            //Physics
            const PhysicSystem* physicSystem_;

    };

}
