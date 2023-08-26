
#include <Mesh/Face.hpp>

#include <Core/Global.hpp>
#include <GraphicObjects/BufferTemplates.hpp>

namespace mav {

	Face::Face(Environment* environment, Material material, float size, glm::vec3 position)
        : Mesh(8, environment, material, size, position) {}

    void Face::generateVertices() {}

    void Face::generateVertices(std::array<glm::vec3, 4> const& points){

        size_t nbOfData = 8;

        size_t verticesNb = 4*nbOfData; //Number of face * number of vertices per face * number of information per vertice
        size_t indicesSize_ = 2*3; //Number of triangle * number of point per triangle

        vertices_.resize(verticesNb);
        indices_.resize(indicesSize_);

        glm::vec3 color(0, 1, 0);

        //TODO: Declare this upper and use it in face, voxel, debug voxel...
        static const std::vector<std::pair<float, float>> verticesTexturesPositions {
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f}
        };

        for(size_t j = 0; j < 4; ++j){

            //We also add (vertice index * nbOfData) to get current vertice position
            size_t verticeOffset = j*nbOfData;

            //Position
            vertices_[verticeOffset + 0 + 0] = points[j].x;
            vertices_[verticeOffset + 0 + 1] = points[j].y;
            vertices_[verticeOffset + 0 + 2] = points[j].z;

            //Normals
            vertices_[verticeOffset + 3 + 0] = 0;
            vertices_[verticeOffset + 3 + 1] = 0;
            vertices_[verticeOffset + 3 + 2] = 0;
            //vertices_[verticeOffset + 3 + fixedIndex] = fixedValue;

            //Color
            vertices_[verticeOffset + 6 + 0] = verticesTexturesPositions[j].first;
            vertices_[verticeOffset + 6 + 1] = verticesTexturesPositions[j].second;

        }

        //We calculate the indices
        //First triangle
        indices_[0] = 0;
        indices_[1] = 1;
        indices_[2] = 3;
        
        //Second triangle
        indices_[3] = 2;
        indices_[4] = 3;
        indices_[5] = 1;

    }

    std::vector<uint32_t> Face::getVertexAttributesSizes() const {
        return {3, 3, 2};
    }

    void Face::updateShader(vuw::Shader* shader, uint32_t currentFrame) {
        
        //Binding 0
		ModelViewProjectionObject mvp{};
		mvp.model = glm::mat4(1.0f);
		mvp.view = environment_->camera->GetViewMatrix();
		mvp.projection = environment_->camera->Projection;
		mvp.projection[1][1] *= -1;

		mvp.modelNormal = glm::transpose(glm::inverse(mvp.model));
		// mvp.modelNormal = glm::mat3(1.0f);
		// glm::mat3(glm::transpose(glm::inverse(model))

		//Binding 1
		float totalTime = environment_->totalElapsedTime;

		shader->updateUniform(0, currentFrame, &mvp, sizeof(mvp));
		shader->updateUniform(1, currentFrame, &totalTime, sizeof(totalTime));

    }

}
