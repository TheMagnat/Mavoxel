
#include <GLObject/Drawable.hpp>
// #include <Core/Global.hpp>

#include <Environment/Environment.hpp>


namespace mav {

    class Quad : public Drawable {

        public:

            Quad(Environment* environment)
                : environment_(environment) {}

            
            std::vector<uint32_t> getVertexAttributesSizes() const override {
                return {{3}, {2}};
            }

            virtual void updateShader(vuw::Shader* shader, uint32_t currentFrame) const override {
                //TODO
            }


            void generateVertices() override {

                vertices_= {
                    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // bottom left 
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // top left
                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // top right
                    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  // bottom right
                };

                //Clockwise
                // indices_ = {
                //     0, 1, 3, //First triangle
                //     2, 3, 1 //Second triangle
                // };

                //Counter clockwise
                indices_ = {
                    0, 3, 1, //First triangle
                    2, 1, 3 //Second triangle
                };

            }


        protected:
            //Environment
            Environment* environment_;

    };

}