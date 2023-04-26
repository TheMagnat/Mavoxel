
#include <GLObject/Drawable.hpp>

#include <Helper/Benchmark/Profiler.hpp>

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

        #ifdef TIME
			Profiler profiler("Graphic update (in Drawable)");
		#endif

        vao_.setAll(vertices_, attributesSum_, attributes_, indices_);
        indicesSize_ = indices_.size();
        // vertices_.clear();
        // indices_.clear();
    }

}
