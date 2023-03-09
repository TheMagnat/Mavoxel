
#include <World/SimpleVoxel.hpp>
#include <cstddef>

namespace mav {

    //Static members
    std::array<uint8_t, 6> SimpleVoxel::verticesIndices = {0, 1, 3, 2, 3, 1};
    std::array<std::vector<float>, 6> SimpleVoxel::generalfacesVertices = std::array<std::vector<float>, 6>();

    void SimpleVoxel::generateGeneralFaces(float voxelSize) {

        // Static variables of the method

        /**
         * This vector store a pair for each face that tell which axe is fixed (first) and what's its value (second).
        */
        static const std::vector<std::pair<size_t, float>> faceFixedValue {
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
        static const std::vector<std::pair<std::vector<size_t>, std::vector<float>>> faceAlternateValues {
            {{0, 2}, {-1, 1}}, //bottom
            {{0, 1}, {-1, 1}}, //front
            {{2, 1}, {1, 1}}, //right
            {{0, 1}, {1, 1}}, //back
            {{2, 1}, {-1, 1}}, //left
            {{0, 2}, {-1, -1}} //top
        };

        size_t nbDataInVertices = 7; //3 positions, 3 normals, 1 id
        size_t nbVerticesPerFace = 4 * nbDataInVertices; //number of vertices per face * number of information per vertice

        // Prepare faces size
        for(std::vector<float>& face : generalfacesVertices){ face.resize(nbVerticesPerFace); }

        float verticeLenght = 0.5f * voxelSize;
        for (size_t i = 0; i < 6; ++i) {

            //We will fill each face
            std::vector<float>& currentFace = generalfacesVertices[i];

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

                //Position (Offset should be add after on copy)
                currentFace[verticeOffset + fixedIndex]  = fixedValue * verticeLenght;
                currentFace[verticeOffset + secondIndex] = secondValue * verticeLenght;
                currentFace[verticeOffset + firstIndex]  = firstValue * verticeLenght;

                //Normals
                currentFace[verticeOffset + 3 + 0] = 0;
                currentFace[verticeOffset + 3 + 1] = 0;
                currentFace[verticeOffset + 3 + 2] = 0;
                currentFace[verticeOffset + 3 + fixedIndex] = fixedValue;

                //id
                currentFace[verticeOffset + 6 + 0] = 0; // Default value to set

                //Here we change first value
                if (j % 2 == 0) {
                    firstValue *= -1;
                }

                if (j == 1) {
                    secondValue *= -1;
                }

            }

        }

    }




    // Non static members
    SimpleVoxel::SimpleVoxel(glm::vec3 const& position, int id, float size)
        : id_(id), size_(size), position_(position) {

        //At the start all faces are not shown
        stateOfFaces_.fill(true);

    }

    //TODO: Mayve delete ? 
    // void SimpleVoxel::generateFaces() {

    //     size_t nbDataInVertices = 7; //3 positions, 3 normals, 1 id

    //     size_t nbVerticesPerFace = 4*nbDataInVertices; //number of vertices per face * number of information per vertice

    //     for(std::vector<float>& face : faces_){
    //         face.resize(nbVerticesPerFace);
    //     }

    //     float verticeLenght = 0.5f * size_;

    //     std::vector<std::pair<size_t, float>> faceFixedValue {
    //         {1, -1}, //bottom
    //         {2, 1}, //front
    //         {0, 1}, //right
    //         {2, -1}, //back
    //         {0, -1}, //left
    //         {1, 1} //top
    //     };

    //     /**
    //      * This vector store a pair for each face that indicate the order at which each axe need to move.
    //      * The first element in the pair indicate the indexes of the axes and the second element indicate
    //      * the first value of each axe.
    //     */
    //     std::vector<std::pair<std::vector<size_t>, std::vector<float>>> faceAlternateValues {
    //         {{0, 2}, {-1, 1}}, //bottom
    //         {{0, 1}, {-1, 1}}, //front
    //         {{2, 1}, {1, 1}}, //right
    //         {{0, 1}, {1, 1}}, //back
    //         {{2, 1}, {-1, 1}}, //left
    //         {{0, 2}, {-1, -1}} //top
    //     };
        
    //     for (size_t i = 0; i < 6; ++i) {

    //         //We will fill each face
    //         std::vector<float>& currentFace = faces_[i];

    //         //We calculate the vertices
    //         size_t fixedIndex = faceFixedValue[i].first;
    //         float fixedValue = faceFixedValue[i].second;

    //         size_t secondIndex = faceAlternateValues[i].first[0];
    //         size_t firstIndex  = faceAlternateValues[i].first[1];

    //         float secondValue = faceAlternateValues[i].second[0];
    //         float firstValue = faceAlternateValues[i].second[1];

    //         for(size_t j = 0; j < 4; ++j) {

    //             //We also add (vertice index * nbDataInVertices) to get current vertice position
    //             size_t verticeOffset = j*nbDataInVertices;

    //             //Position
    //             currentFace[verticeOffset + fixedIndex]  = position_[fixedIndex] + fixedValue * verticeLenght;
    //             currentFace[verticeOffset + secondIndex] = position_[secondIndex] + secondValue * verticeLenght;
    //             currentFace[verticeOffset + firstIndex]  = position_[firstIndex] + firstValue * verticeLenght;

    //             //Normals
    //             currentFace[verticeOffset + 3 + 0] = 0;
    //             currentFace[verticeOffset + 3 + 1] = 0;
    //             currentFace[verticeOffset + 3 + 2] = 0;
    //             currentFace[verticeOffset + 3 + fixedIndex] = fixedValue;

    //             //id
    //             currentFace[verticeOffset + 6 + 0] = id_;

    //             //Here we change first value
    //             if (j % 2 == 0) {
    //                 firstValue *= -1;
    //             }

    //             if (j == 1) {
    //                 secondValue *= -1;
    //             }

    //         }

    //     }

    //     std::cout << "Finished" << std::endl; //TODO: Remove

    // }

    // void SimpleVoxel::generateFace(uint8_t faceIndex) {

    //     static size_t nbDataInVertices = 7; //3 positions, 3 normals, 1 id

    //     faces_[faceIndex] = SimpleVoxel::generalfacesVertices[faceIndex];

    //     // We will copy the face at faceIndex
    //     std::vector<float>& currentFace = faces_[faceIndex];

    //     for(size_t j = 0; j < 4; ++j) {

    //         //We also add (vertice index * nbDataInVertices) to get current vertice position
    //         size_t verticeOffset = j*nbDataInVertices;

    //         //Position
    //         currentFace[verticeOffset + 0] += position_.x;
    //         currentFace[verticeOffset + 1] += position_.y;
    //         currentFace[verticeOffset + 2] += position_.z;

    //         //id
    //         currentFace[verticeOffset + 6 + 0] = id_;

    //     }


    // }

    
    // std::array<std::vector<float>, 6> const& SimpleVoxel::getFaces() const {
    //     return faces_;
    // }

    std::vector<float> SimpleVoxel::getFace(uint8_t faceIndex) const {

        static size_t nbDataInVertices = 7; //3 positions, 3 normals, 1 id

        // We will copy the face at faceIndex
        std::vector<float> face = SimpleVoxel::generalfacesVertices[faceIndex];


        for(size_t j = 0; j < 4; ++j) {

            //We also add (vertice index * nbDataInVertices) to get current vertice position
            size_t verticeOffset = j*nbDataInVertices;

            //Position
            face[verticeOffset + 0] += position_.x;
            face[verticeOffset + 1] += position_.y;
            face[verticeOffset + 2] += position_.z;

            //id
            face[verticeOffset + 6 + 0] = id_;

        }

        return face;

    }

    void SimpleVoxel::setFaceState(uint8_t faceIndex, bool state) {
        stateOfFaces_[faceIndex] = state;
    }

    bool SimpleVoxel::getFaceState(uint8_t faceIndex) const {
        return stateOfFaces_[faceIndex];
    }

}