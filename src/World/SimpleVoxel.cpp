
#include <World/SimpleVoxel.hpp>
#include <cstddef>

namespace mav {

    std::array<uint8_t, 6> SimpleVoxel::verticesIndices = {0, 1, 3, 2, 3, 1};

    SimpleVoxel::SimpleVoxel(glm::vec3 const& position, int id)
        : id_(id), position_(position) {

        //At the start all faces are not shown
        stateOfFaces_.fill(false);

        generateFaces();

    }

    void SimpleVoxel::generateFaces() {

        size_t nbDataInVertices = 7; //3 positions, 3 normals, 1 id

        size_t verticesNbPerFace = 4*nbDataInVertices; //Number of face * number of vertices per face * number of information per vertice
        size_t indicesNbPerFace = 2*3; //Number of face * number of triangle * number of vertice per triangle

        for(std::vector<float>& face : faces_){
            face.resize(verticesNbPerFace);
        }

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
        
        for (size_t i = 0; i < 6; ++i) {

            //We will fill each face
            std::vector<float>& currentFace = faces_[i];

            //We calculate the vertices
            size_t fixedIndex = faceFixedValue[i].first;
            float fixedValue = faceFixedValue[i].second;

            size_t secondIndex = faceAlternateValues[i].first[0];
            size_t firstIndex  = faceAlternateValues[i].first[1];

            float secondValue = faceAlternateValues[i].second[0];
            float firstValue = faceAlternateValues[i].second[1];

            for(size_t j = 0; j < 4; ++j) {

                //We also add (vertice index * nbDataInVertices) to get current vertice position
                size_t verticeOffset = j*nbDataInVertices;

                //Position
                //TODO: Do something with the position here
                currentFace[verticeOffset + fixedIndex] = fixedValue * verticeLenght;
                currentFace[verticeOffset + secondIndex] = secondValue * verticeLenght;
                currentFace[verticeOffset + firstIndex] = firstValue * verticeLenght;

                //Normals
                currentFace[verticeOffset + 3 + 0] = 0;
                currentFace[verticeOffset + 3 + 1] = 0;
                currentFace[verticeOffset + 3 + 2] = 0;
                currentFace[verticeOffset + 3 + fixedIndex] = fixedValue;

                //id
                currentFace[verticeOffset + 6 + 0] = id_;

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
            /*
            indices_[i*6 + 0] = i*4 + 0;
            indices_[i*6 + 1] = i*4 + 1;
            indices_[i*6 + 2] = i*4 + 3;
            
            //Second triangle
            indices_[i*6 + 3] = i*4 + 2;
            indices_[i*6 + 4] = i*4 + 3;
            indices_[i*6 + 5] = i*4 + 1;
            */

        }

    }

    std::array<std::vector<float>, 6> const& SimpleVoxel::getFaces() const {
        return faces_;
    }

    void SimpleVoxel::setFaceState(uint8_t faceIndex, bool state) {
        stateOfFaces_[faceIndex] = state;
    }

    bool SimpleVoxel::getFaceState(uint8_t faceIndex) const {
        return stateOfFaces_[faceIndex];
    }

}