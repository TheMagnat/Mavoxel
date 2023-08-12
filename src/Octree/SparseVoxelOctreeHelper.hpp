
#pragma once

#include <World/WorldPositionHelper.hpp>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <cmath>

namespace mav {

    struct SVOCollisionInformation {
        uint32_t id;
        glm::uvec3 position;
    };

    inline glm::ivec3 getIntPosition(glm::vec3 const& position, glm::vec3 const& directionSign) {

        return directionSign * glm::floor(directionSign * position) - (glm::vec3)glm::lessThan(directionSign, glm::vec3(0));

    }

    inline glm::vec3 computeDistanceToSide(glm::vec3 const& position, glm::vec3 const& direction, glm::vec3 const& directionSign, float leafSize) {

        //This convert position to leaf sized position (by dividing with leaf size) and then calculate it's world discrete position, then put it back to normal sized position (by multiplying by leaf size)
        glm::vec3 nextPosition = (directionSign * glm::floor(directionSign * position / leafSize) + directionSign) * leafSize;
        glm::vec3 distTotal = (nextPosition - position) / direction;

        return distTotal;

    }

    inline uint8_t argMin(glm::vec3 const& vec) {
        
        int index = 0;
        float minValue = vec.x;

        if (vec.y < minValue) {
            index = 1;
            minValue = vec.y;
        }

        if (vec.z < minValue) {
            index = 2;
        }

        return index;
    }

    inline std::pair<float, uint8_t> getShortestTraveledDistanceAndIndex(glm::vec3 const& position, glm::vec3 const& direction, glm::vec3 const& directionSign, float leafSize) {
        //Required time to exit the full voxel along each axis.
        glm::vec3 tMax = computeDistanceToSide(position, direction, directionSign, leafSize);
        
        glm::vec3 lowestIndexVec = glm::step(tMax, glm::vec3(tMax.y, tMax.z, tMax.x));
        lowestIndexVec *= (glm::vec3(1.0) - glm::vec3(lowestIndexVec.z, lowestIndexVec.x, lowestIndexVec.y));

        uint8_t lowestIndex = uint8_t(glm::dot(lowestIndexVec, glm::vec3(0, 1, 2)));

        return { tMax[lowestIndex], lowestIndex };
    }

}
