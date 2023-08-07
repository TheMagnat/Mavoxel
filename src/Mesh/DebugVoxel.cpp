
#include <Mesh/DebugVoxel.hpp>

#include <Core/Global.hpp>
#include <GraphicObjects/BufferTemplates.hpp>

namespace mav {

	DebugVoxel::DebugVoxel(Environment* environment, Material material, float size, glm::vec3 position)
        : Mesh(3, environment, material, size, position), color_(1.0f, 1.0f, 1.0f) {}

    void DebugVoxel::generateVertices() {

        size_t nbOfData = 3;

        size_t verticesNb = 6*4*nbOfData; //Number of face * number of vertices per face * number of information per vertice
        size_t indicesSize_ = 6*4*2; //Number of face * number of line * number of vertice per line

        vertices_.resize(verticesNb);
        indices_.resize(indicesSize_);

        glm::vec3 color(0, 1, 0);
        float verticeLength = 0.5f;


        std::vector<std::pair<size_t, float>> faceFixedValue {
            {1, -1.0f}, //bottom
            {2, 1.0f}, //front
            {0, 1.0f}, //right
            {2, -1.0f}, //back
            {0, -1.0f}, //left
            {1, 1.0f} //top
        };

        /**
         * This vector store a pair for each face that indicate the order at which each axe need to move.
         * The first element in the pair indicate the indexes of the axes and the second element indicate
         * the first value of each axe.
        */
        std::vector<std::pair<std::vector<size_t>, std::vector<float>>> faceAlternateValues {
            {{0, 2}, {-1, 1}}, //bottom
            {{0, 1}, {-1, 1}}, //front
            {{2, 1}, {1, 1}}, //right
            {{0, 1}, {1, 1}}, //back
            {{2, 1}, {-1, 1}}, //left
            {{0, 2}, {-1, -1}} //top
        };
        
        for (size_t i = 0; i < 6; ++i)
        {

            //We calculate the vertices
            size_t fixedIndex = faceFixedValue[i].first;
            float fixedValue = faceFixedValue[i].second;

            size_t secondIndex = faceAlternateValues[i].first[0];
            size_t firstIndex  = faceAlternateValues[i].first[1];

            float secondValue = faceAlternateValues[i].second[0];
            float firstValue = faceAlternateValues[i].second[1];

            //Indice start for each face (face index * number of vertice per face * number of information per vertice)
            size_t faceOffset = i*4*nbOfData;

            for(size_t j = 0; j < 4; ++j){

                //We also add (vertice index * nbOfData) to get current vertice position
                size_t verticeOffset = j*nbOfData;

                //Position
                vertices_[faceOffset + verticeOffset + fixedIndex] = fixedValue * verticeLength;
                vertices_[faceOffset + verticeOffset + secondIndex] = secondValue * verticeLength;
                vertices_[faceOffset + verticeOffset + firstIndex] = firstValue * verticeLength;

                //Here we change first value
                if (j % 2 == 0) {
                    firstValue *= -1;
                }

                if (j == 1) {
                    secondValue *= -1;
                }

            }

            //We calculate the indices

            //Each face have 6 indices
            //First triangle
            indices_[i*8 + 0] = i*4 + 0;
            indices_[i*8 + 1] = i*4 + 1;

            indices_[i*8 + 2] = i*4 + 1;
            indices_[i*8 + 3] = i*4 + 2;

            indices_[i*8 + 4] = i*4 + 2;
            indices_[i*8 + 5] = i*4 + 3;

            indices_[i*8 + 6] = i*4 + 3;
            indices_[i*8 + 7] = i*4 + 0;

        }

    }

    void DebugVoxel::setColor(glm::vec3 const& color) {
        color_ = color;
    }

    std::vector<uint32_t> DebugVoxel::getVertexAttributesSizes() const {
        return {3};
    }

    void DebugVoxel::updateShader(vuw::Shader* shader, uint32_t currentFrame) const {
        
        //Binding 0
        ModelViewProjectionObjectNoNormal mvp{};
        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(size));

        glm::mat4 model2(1.0f);
        model2 = model2 * translationMatrix_;
        model2 = glm::scale(model2, glm::vec3(size));

        glm::mat4 model3(1.0f);
        model3 = glm::translate(model3, position);
        model3 = glm::scale(model3, glm::vec3(size));


        mvp.model = model;
        mvp.view = environment_->camera->GetViewMatrix();
        mvp.projection = environment_->camera->Projection;
        mvp.projection[1][1] *= -1;

        shader->updateUniform(0, currentFrame, &mvp, sizeof(mvp));
        shader->updateUniform(1, currentFrame, (void*)&color_, sizeof(color_));

    }

}