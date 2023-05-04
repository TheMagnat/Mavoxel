
#include <GLObject/Drawable.hpp>

#include <Helper/Benchmark/Profiler.hpp>

namespace mav {

    Drawable::Drawable(size_t attributesSum, std::vector<VAO::Attribute> const& attributes, Shader* shader)
        : shader_(shader), attributesSum_(attributesSum), attributes_(attributes) {
            vao_.init(true);
        }

    void Drawable::initialize() {
        generateVertices();
        graphicUpdate();
    }

    void Drawable::graphicUpdate(){

        #ifdef TIME
			Profiler profiler("Graphic update (in Drawable)");
		#endif

        vao_.setAll(vertices_, attributesSum_, attributes_, indices_);
        indicesSize_ = (int)indices_.size();
        // vertices_.clear();
        // indices_.clear();
    }

}
