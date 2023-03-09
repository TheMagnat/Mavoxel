
#include <array>
#include <vector>
#include <glm/vec3.hpp>

namespace mav {

    class SimpleVoxel {
        public:

            // This array contain the correct sequence to convert face vertices to 2 openGL triangles
            static std::array<uint8_t, 6> verticesIndices;

            // This array will contain the faces basics vertices
            static std::array<std::vector<float>, 6> generalfacesVertices;
            static void generateGeneralFaces(float voxelSize);

            // Faces indexes in the faces array
            enum Face { bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5 };

            SimpleVoxel(glm::vec3 const& position, int id = 0, float size = 1);
            
            //TODO: Maybe delete ?
            //void generateFaces();
            //void generateFace(uint8_t faceIndex);
            //std::array<std::vector<float>, 6> const& getFaces() const;

            std::vector<float> getFace(uint8_t faceIndex) const;


            void setFaceState(uint8_t faceIndex, bool state = true);
            bool getFaceState(uint8_t faceIndex) const;

        private:
            int id_;
            float size_;

            glm::vec3 position_;

            //TODO: Maybe delete ?
            //std::array<std::vector<float>, 6> faces_;
            std::array<bool, 6> stateOfFaces_;

    };

}