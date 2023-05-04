
#pragma once

#include <glm/vec3.hpp>

#include <Collision/AABB.hpp>
#include <Physics/Gravity.hpp>
#include <World/World.hpp>

#include <Mesh/DebugVoxel.hpp>


#define GRAVITY_ON false

namespace mav {

    enum class Direction {
        FRONT,
        UP,
        RIGHT
    };

    class Entity {

        public:
        //TODO: peut être avoir un bool qui vérifie qu'on peut utiliser front et right ?
            Entity(AABB boundingBox);

            void setGravity(const Gravity* newGravity);


            //Movement logic
            void addVelocity(Direction direction, float strength, float deltaTime);
            
            /**
             * Direction: binary representation of the direction to apply the strength :
             *      001 -> Front
             *      010 -> Right
             *      100 -> Up
            */
            void addVelocity(glm::vec3 const& direction, float strength, float deltaTime);

            void jump(float strength);

            void groundHit();

            void groundLeft();
            
            void update(float elapsedTime);
            bool update(float elapsedTime, World const& world);
            
            void draw() const;

        public:
            glm::vec3 velocity;
            bool freeFlight = !GRAVITY_ON;
            bool onTheGround = false;

            //Jump information
            int remainingJumps = 0;
            int maxJumps = 2;
            float timeSinceLastJump = 0;
            float timeLimitBetweenJumps = 0.25;

        protected:

            const glm::vec3* front_;
            const glm::vec3* right_;
            
            //Collision
            AABB boundingBox_;

            //Physics
            const Gravity* gravity_;

            //World data


            #ifndef NDEBUG
				DebugVoxel entityBox;
    		#endif

    };

}
