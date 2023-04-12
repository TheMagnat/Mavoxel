
#include <glm/vec3.hpp>
#include <array>

namespace mav {

    struct CollisionFace {

        CollisionFace(std::array<glm::vec3, 4> const& pointsP) : points(pointsP) {}
        CollisionFace(std::vector<float> const& vertices, float distanceP) : distance(distanceP) {
            
            //Data size in vertices
            for(size_t i = 0, j = 0; i < 4; ++i, j += 9) {
                points[i].x = vertices[j + 0];
                points[i].y = vertices[j + 1];
                points[i].z = vertices[j + 2];
            }

            normal.x = vertices[3 + 0];
            normal.y = vertices[3 + 1];
            normal.z = vertices[3 + 2];

        }

        std::array<glm::vec3, 4> points;
        glm::vec3 normal;

        float distance;

    };

}
