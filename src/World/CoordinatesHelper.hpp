
#pragma once

#include <glm/glm.hpp>


namespace std {
    // Define a hash function for glm::ivec3
    template <>
    struct hash<glm::ivec3> {
        size_t operator()(const glm::ivec3& coords) const {
            constexpr int PRIME_1 = 73856093;
            constexpr int PRIME_2 = 19349663;
            constexpr int PRIME_3 = 83492791;
            
            // Map coordinates to a smaller range
            int x = coords.x % PRIME_1;
            int y = coords.y % PRIME_2;
            int z = coords.z % PRIME_3;
            
            // Hash the coordinates
            std::size_t x_hash = std::hash<int>()(x);
            std::size_t y_hash = std::hash<int>()(y);
            std::size_t z_hash = std::hash<int>()(z);
            
            // Combine the hashes
            return x_hash ^ (y_hash << 1) ^ (z_hash << 2);
        }
    };

}
