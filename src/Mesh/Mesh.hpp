#pragma once

#include <GraphicObjects/Camera.hpp>
#include <GraphicObjects/Drawable.hpp>

#include <Environment/Environment.hpp>

#include <Material/Material.hpp>

#include <glm/glm.hpp>

#include <vector>

namespace mav {

    struct Environment;
    
    //TODO: Utiliser drawable
    class Mesh : public Drawable {

        public:

            //TODO: retirer cette dépendant à un material qui est peu utilisé
            Mesh(size_t attributesSum, Environment* environment, Material material, float size = 1, glm::vec3 position = glm::vec3(0.0f));


            void setPosition(glm::vec3 const& newPosition);
            void setPosition(float x, float y, float z);
            void updatePosition();

            glm::vec3 const& getPosition() const;

        public:

            //Model information
            float size;
            glm::vec3 position;
            Material material;


        protected:

            //Model data
            glm::mat4 translationMatrix_;

            //Environment
            Environment* environment_;


    };

}