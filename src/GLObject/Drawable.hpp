
#include <GLObject/Shader.hpp>
#include <GLObject/GLObject.hpp>

#include <vector>

namespace mav {

    class Drawable {

        public:
            Drawable(bool shouldInitialize, size_t attributesSum, std::vector<VAO::Attribute> const& attributes, Shader* shader);
            
            //To initialize the VAO
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

        };

}