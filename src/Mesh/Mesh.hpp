#pragma once

#include <GLObject/Shader.hpp>
#include <GLObject/Camera.hpp>
#include <GLObject/GLObject.hpp>

#include <Environment/Environment.hpp>

#include <Material/Material.hpp>

#include <glm/glm.hpp>

#include <vector>


namespace mav {

    struct Environment;


    class Mesh {

        public:

            Mesh(Shader* shaderPtr, Environment* environment, Material material, float size = 1, glm::vec3 position = glm::vec3(0.0f));

            virtual void init();
            virtual void updateVAO() = 0;

            void setPosition(glm::vec3 newPosition);
            void setPosition(float x, float y, float z);
            void updatePosition();

            glm::vec3 getPosition();

            virtual void draw();


        public:

            //Model information
            float size;
            glm::vec3 position;
            Material material;


        protected:

            //Model data
            glm::mat4 translationMatrix_;

            //Environement
            Environment* environment_;

            //GL
            Shader* shaderPtr_;

            VAO vao_;
            std::vector<float> vertices_;

            size_t indicesNb_;
            std::vector<int> indices_;

    };

}