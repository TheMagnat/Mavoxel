#pragma once

#include <GLObject/Shader.hpp>
#include <GLObject/GLObject.hpp>

#include <vector>

namespace mav {

    class Drawable {

        public:
            Drawable(size_t attributesSum, std::vector<VAO::Attribute> const& attributes, Shader* shader);

            //To generate vertices
            virtual void generateVertices() = 0;

            /**
             * To initialize the VAO.
             * If fullInit is set to true, also generate vertices and call graphic update to set graphic buffer data
            */
            void initialize();

            //To update the VAO buffers
            void graphicUpdate();

        protected:
            //OpenGL
            Shader* shader_;

            VAO vao_;
            size_t attributesSum_;
            std::vector<VAO::Attribute> attributes_;

            std::vector<float> vertices_;
            
            std::vector<int> indices_;
            int indicesSize_ = 0;

        };

}