
#include <GLObject/Drawable.hpp>

#include <Core/Global.hpp>

#include <Helper/Benchmark/Profiler.hpp>

namespace mav {

    Drawable::Drawable() : vertexData_(Global::vulkanWrapper->generateVertexData()) {


    }

    bool Drawable::empty() {
        return indices_.empty();
    }

    void Drawable::initialize() {
        // vao_.init(true);

        generateVertices();
        graphicUpdate();

    }

    void Drawable::graphicUpdate(){

        #ifdef TIME
			Profiler profiler("Graphic update (in Drawable)");
		#endif

        //Old openGL
        // vao_.setAll(vertices_, attributesSum_, attributes_, indices_);
        // indicesSize_ = indices_.size();

        vertexData_.setData(vertices_, indices_);
        // vertices_.clear();
        // indices_.clear();
    }

    void Drawable::draw(VkCommandBuffer commandBuffer) const {
        vertexData_.bind(commandBuffer);
		vertexData_.draw(commandBuffer);
    }

}
