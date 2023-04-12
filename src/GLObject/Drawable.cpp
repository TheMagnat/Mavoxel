
#include <GLObject/Drawable.hpp>

namespace mav {

    Drawable::Drawable(bool shouldInitialized, size_t attributesSum, std::vector<VAO::Attribute> const& attributes, Shader* shader)
        : shader_(shader), attributesSum_(attributesSum), attributes_(attributes) {

        if (shouldInitialized) {
            initialize(false);
        }

    }

    void Drawable::initialize(bool fullInit) {
        vao_.init(true);

        if (fullInit) {
            generateVertices();
            graphicUpdate();
        }

    }

    void Drawable::graphicUpdate(){
        vao_.setAll(vertices_, attributesSum_, attributes_, indices_);
        indicesSize_ = indices_.size();
        // vertices_.clear();
        // indices_.clear();
    }

}
