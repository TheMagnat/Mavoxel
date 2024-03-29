
#include <World/SimpleVoxel.hpp>
#include <cstddef>

//3 positions, 3 normals, 2 textures, 1 ambient occlusion, 1 id
static const size_t nbDataInVertices = 3+3+2+1+1;

namespace mav {

    //Static members
    std::array<uint8_t, 6> SimpleVoxel::verticesIndices = {0, 1, 3, 2, 3, 1};
    std::array<uint8_t, 6> SimpleVoxel::flippedVerticesIndices = {1, 2, 0, 3, 0, 2};
    std::array<std::vector<float>, 6> SimpleVoxel::generalFacesVertices = std::array<std::vector<float>, 6>();

    void SimpleVoxel::generateGeneralFaces(float voxelSize) {

        // Static variables of the method

        /**
         * This vector store a pair for each face that tell which axe is fixed (first) and what's its value (second).
        */
        static const std::vector<std::pair<size_t, float>> faceFixedValue {
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
        static const std::vector<std::pair<std::vector<size_t>, std::vector<float>>> faceAlternateValues {
            {{0, 2}, {-1, 1}}, //bottom
            {{0, 1}, {-1, 1}}, //front
            {{2, 1}, {1, 1}}, //right
            {{0, 1}, {1, 1}}, //back
            {{2, 1}, {-1, 1}}, //left
            {{0, 2}, {-1, -1}} //top
        };

        //top left, bottom left, bottom right, top right
        static const std::vector<std::pair<float, float>> verticesTexturesPositions {
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f}
        };

        size_t nbVerticesPerFace = 4 * nbDataInVertices; //number of vertices per face * number of information per vertice

        // Prepare faces size
        for(std::vector<float>& face : generalFacesVertices){ face.resize(nbVerticesPerFace); }

        float verticesLength = 0.5f * voxelSize;
        for (size_t i = 0; i < 6; ++i) {

            //We will fill each face
            std::vector<float>& currentFace = generalFacesVertices[i];

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
                currentFace[verticeOffset + fixedIndex]  = fixedValue * verticesLength;
                currentFace[verticeOffset + secondIndex] = secondValue * verticesLength;
                currentFace[verticeOffset + firstIndex]  = firstValue * verticesLength;

                //Normals
                currentFace[verticeOffset + 3 + 0] = 0;
                currentFace[verticeOffset + 3 + 1] = 0;
                currentFace[verticeOffset + 3 + 2] = 0;
                currentFace[verticeOffset + 3 + fixedIndex] = fixedValue;

                //Textures
                currentFace[verticeOffset + 6 + 0] = verticesTexturesPositions[j].first;
                currentFace[verticeOffset + 6 + 1] = verticesTexturesPositions[j].second;

                //Ambient occlusion
                currentFace[verticeOffset + 8 + 0] = 1.0f;

                //Id
                currentFace[verticeOffset + 9 + 0] = 0; // Default value set to id 0

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
    SimpleVoxel::SimpleVoxel(glm::vec3 const& position, glm::ivec3 const& chunkPosition, int id)
        : id_(id), position_(position), chunkPosition_(chunkPosition) {

        //At the start all faces are not shown
        stateOfFaces_.fill(true);

    }

    std::vector<float> SimpleVoxel::getFace(uint8_t faceIndex) const {

        // We will copy the face at faceIndex
        std::vector<float> face = SimpleVoxel::generalFacesVertices[faceIndex];


        for(size_t j = 0; j < 4; ++j) {

            //We also add (vertice index * nbDataInVertices) to get current vertice position
            size_t verticeOffset = j*nbDataInVertices;

            //Position
            face[verticeOffset + 0] += position_.x;
            face[verticeOffset + 1] += position_.y;
            face[verticeOffset + 2] += position_.z;

            //id
            face[verticeOffset + 9 + 0] = id_;

        }

        return face;

    }

    std::vector<float> SimpleVoxel::getFace(uint8_t faceIndex, std::array<float, 4> const& ambientOcclusion) const {

        // We will copy the face at faceIndex
        std::vector<float> face = SimpleVoxel::generalFacesVertices[faceIndex];


        for(size_t j = 0; j < 4; ++j) {

            //We also add (vertice index * nbDataInVertices) to get current vertice position
            size_t verticeOffset = j*nbDataInVertices;

            //Position
            face[verticeOffset + 0] += position_.x;
            face[verticeOffset + 1] += position_.y;
            face[verticeOffset + 2] += position_.z;

            //Ambient Occlusion
            face[verticeOffset + 8 + 0] = ambientOcclusion[j];

            //id
            face[verticeOffset + 9 + 0] = id_;

        }

        return face;

    }

    //TODO: Better, generate the normal vector manually as a static variable
    glm::vec3 SimpleVoxel::getFaceNormal(uint8_t faceIndex) {
        
        /**
         * This vector store a pair for each face that tell which axe is fixed (first) and what's its value (second).
         */
        static const std::vector<std::pair<size_t, float>> faceFixedValue {
            {1, -1.0f}, //bottom
            {2, 1.0f}, //front
            {0, 1.0f}, //right
            {2, -1.0f}, //back
            {0, -1.0f}, //left
            {1, 1.0f} //top
        };

        // We will copy the face at faceIndex
        size_t fixedIndex = faceFixedValue[faceIndex].first;
        float fixedValue = faceFixedValue[faceIndex].second;

        glm::vec3 normal(0.0f);
        normal[fixedIndex] = fixedValue;

        return normal;

    }

    void SimpleVoxel::setFaceState(uint8_t faceIndex, bool state) {
        stateOfFaces_[faceIndex] = state;
    }

    bool SimpleVoxel::getFaceState(uint8_t faceIndex) const {
        return stateOfFaces_[faceIndex];
    }

    int SimpleVoxel::getId() const {
        return id_;
    }

    glm::vec3 const& SimpleVoxel::getPosition() const {
        return position_;
    }

    glm::ivec3 const& SimpleVoxel::getChunkPosition() const {
        return chunkPosition_;
    }
}
