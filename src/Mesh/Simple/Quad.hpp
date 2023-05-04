
#include <GLObject/Drawable.hpp>
#include <GLObject/Shader.hpp>
#include <GLObject/Texture3D.hpp>
#include <Core/Global.hpp>

#include <Environment/Environment.hpp>


namespace mav {

    class Quad : public Drawable {

        public:

            Quad(Shader* shaderPtr, Environment* environment)
                : Drawable(5, {{3}, {2}}, shaderPtr), environment_(environment) {}

            void generateVertices() override {

                vertices_= {
                    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // top left 
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
                    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  // top right
                };

                indices_ = {
                    0, 1, 3, //First triangle
                    2, 3, 1 //Second triangle
                };

            }

            void draw() {
                vao_.bind();

                shader_->use();

                glDrawElements(GL_TRIANGLES, indicesSize_, GL_UNSIGNED_INT, 0);
            }


        protected:
            //Environement
            Environment* environment_;

    };

}