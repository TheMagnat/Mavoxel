#pragma once

#include <array>
#include <vector>
#include <glm/vec3.hpp>

#include <Collision/AABB.hpp>

namespace mav {

    class SimpleVoxel {
        public:

            // This array contain the correct sequence to convert face vertices to 2 openGL triangles
            static std::array<uint8_t, 6> verticesIndices;
            // There is a flipped version for a better interpolation of the ambient occlusion values
            static std::array<uint8_t, 6> flippedVerticesIndices;
            
            // This array will contain the faces basics vertices
            static std::array<std::vector<float>, 6> generalFacesVertices;
            static void generateGeneralFaces(float voxelSize);

            // Faces indexes in the faces array
            enum Face { bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5 };

            SimpleVoxel(glm::vec3 const& position, int id = 0);
            
            //TODO: Maybe delete ?
            //void generateFaces();
            //void generateFace(uint8_t faceIndex);
            //std::array<std::vector<float>, 6> const& getFaces() const;

            // Faces informations
            std::vector<float> getFace(uint8_t faceIndex) const;
            std::vector<float> getFace(uint8_t faceIndex, std::array<float, 4> const& ambientOcclusion) const;

            void setFaceState(uint8_t faceIndex, bool state = true);
            bool getFaceState(uint8_t faceIndex) const;

            // Getters
            glm::vec3 const& getPosition() const;

        private:
            int id_;

            glm::vec3 position_;

            std::array<bool, 6> stateOfFaces_;

    };

}