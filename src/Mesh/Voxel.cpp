
#include <Mesh/Voxel.hpp>

#include <Core/Global.hpp>

namespace mav {

	Voxel::Voxel(Environment* environment, Material material, float size, glm::vec3 position)
        : Mesh(8, {{3}, {3}, {2}}, environment, material, size, position) {}


    void Voxel::generateVertices(){

        size_t nbOfData = 8;

        size_t verticesNb = 6*4*nbOfData; //Number of face * number of vertices per face * number of information per vertice
        size_t indicesSize_ = 6*6; //Number of face * number of triangle * number of vertice per triangle

        vertices_.resize(verticesNb);
        indices_.resize(indicesSize_);

        glm::vec3 color(0, 1, 0);
        float verticesLength = 0.5f;


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

        static const std::vector<std::pair<float, float>> verticesTexturesPositions {
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f}
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

            //Index start for each face (face index * number of vertice per face * number of information per vertice)
            size_t faceOffset = i*4*nbOfData;

            for(size_t j = 0; j < 4; ++j){

                //We also add (vertice index * nbOfData) to get current vertice position
                size_t verticeOffset = j*nbOfData;

                //Position
                vertices_[faceOffset + verticeOffset + fixedIndex] = fixedValue * verticesLength;
                vertices_[faceOffset + verticeOffset + secondIndex] = secondValue * verticesLength;
                vertices_[faceOffset + verticeOffset + firstIndex] = firstValue * verticesLength;

                //Normals
                vertices_[faceOffset + verticeOffset + 3 + 0] = 0;
                vertices_[faceOffset + verticeOffset + 3 + 1] = 0;
                vertices_[faceOffset + verticeOffset + 3 + 2] = 0;
                vertices_[faceOffset + verticeOffset + 3 + fixedIndex] = fixedValue;

                //Textures
                vertices_[faceOffset + verticeOffset + 6 + 0] = verticesTexturesPositions[j].first;
                vertices_[faceOffset + verticeOffset + 6 + 1] = verticesTexturesPositions[j].second;


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
            indices_[i*6 + 0] = i*4 + 0;
            indices_[i*6 + 1] = i*4 + 1;
            indices_[i*6 + 2] = i*4 + 3;
            
            //Second triangle
            indices_[i*6 + 3] = i*4 + 2;
            indices_[i*6 + 4] = i*4 + 3;
            indices_[i*6 + 5] = i*4 + 1;

        }

    }

    std::vector<uint32_t> Voxel::getVertexAttributesSizes() const {
        return {3, 3, 2};
    }

    void Voxel::updateShader(vuw::Shader* shader, uint32_t currentFrame) const {
        //TODO
    }

}