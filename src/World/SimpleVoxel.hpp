
#include <array>
#include <vector>
#include <glm/vec3.hpp>

namespace mav {

    class SimpleVoxel {
        public:

            // This array contain the correct sequence to convert face vertices to 2 openGL triangles
            static std::array<uint8_t, 6> verticesIndices;

            // Faces indexes in the faces array
            enum Face { bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5 };

            SimpleVoxel(glm::vec3 const& position, int id = 0);

            void generateFaces();

            std::array<std::vector<float>, 6> const& getFaces() const;

            void setFaceState(uint8_t faceIndex, bool state = true);
            bool getFaceState(uint8_t faceIndex) const;

        private:
            int id_;

            glm::vec3 position_;

            std::array<std::vector<float>, 6> faces_;
            std::array<bool, 6> stateOfFaces_;

    };

}