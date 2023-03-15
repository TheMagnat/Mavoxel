
#include <GLObject/Drawable.hpp>

namespace mav {

    Drawable::Drawable(bool shouldInitialize, size_t attributesSum, std::vector<VAO::Attribute> const& attributes, Shader* shader)
        : shader_(shader), attributesSum_(attributesSum), attributes_(attributes) {

        if (shouldInitialize) {
            initialize();
        }

    }

    void Drawable::initialize() {
        vao_.init(true);
    }

    void Drawable::graphicUpdate(){
        vao_.setAll(vertices_, attributesSum_, attributes_, indices_);
    }

}
