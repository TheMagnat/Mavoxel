
#include <Mesh/Voxel.hpp>

#include <Core/Global.hpp>

namespace mav {

	Voxel::Voxel(Shader* shaderPtr, Environment* environment, Material material, float size, glm::vec3 position)
        : Mesh(shaderPtr, environment, material, size, position) {}

	void Voxel::init(){
		vao_.init(true);
        generateVertices();
        updateVAO();
	}

    void Voxel::generateVertices(){

        size_t verticesNb = 6*4*9; //Number of face * number of vertices per face * number of information per vertice
        indicesNb_ = 6*6; //Number of face * number of triangle * number of vertice per triangle

        vertices_.resize(verticesNb);
        indices_.resize(indicesNb_);

        glm::vec3 color(0, 1, 0);
        float verticeLenght = 0.5f;


        std::vector<std::pair<size_t, float>> faceFixedValue {
            {1, -1}, //bottom
            {2, 1}, //front
            {0, 1}, //right
            {2, -1}, //back
            {0, -1}, //left
            {1, 1} //top
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
            size_t faceOffset = i*4*9;

            for(size_t j = 0; j < 4; ++j){

                //We also add (vertice index * 9) to get current vertice position
                size_t verticeOffset = j*9;

                //Position
                vertices_[faceOffset + verticeOffset + fixedIndex] = fixedValue * verticeLenght;
                vertices_[faceOffset + verticeOffset + secondIndex] = secondValue * verticeLenght;
                vertices_[faceOffset + verticeOffset + firstIndex] = firstValue * verticeLenght;

                //Normals
                vertices_[faceOffset + verticeOffset + 3 + 0] = 0;
                vertices_[faceOffset + verticeOffset + 3 + 1] = 0;
                vertices_[faceOffset + verticeOffset + 3 + 2] = 0;
                vertices_[faceOffset + verticeOffset + 3 + fixedIndex] = fixedValue;

                //Color
                vertices_[faceOffset + verticeOffset + 6 + 0] = color.r;
                vertices_[faceOffset + verticeOffset + 6 + 1] = color.g;
                vertices_[faceOffset + verticeOffset + 6 + 2] = color.b;

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

        indicesNb_ = indices_.size();
    }

    void Voxel::updateVAO(){

        std::vector<VAO::Attribute> allAttribute = {{3}, {3}, {3}};

		vao_.setAll(vertices_, 9, allAttribute, indices_);
    }


}