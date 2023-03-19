
#include <functional>
#include <cstddef>
#include <limits>

namespace mav {

    struct ChunkCoordinates {
        
        ChunkCoordinates(int xPos, int yPos, int zPos) : x(xPos), y(yPos), z(zPos) {} 

        int x;
        int y;
        int z;


        bool operator==(ChunkCoordinates const& toCompare) const { 
            return (x == toCompare.x && y == toCompare.y && z == toCompare.z);
        }


    };

}

namespace std {
    // Define a hash function for ChunkCoordinates
    template <>
    struct hash<mav::ChunkCoordinates> {
        size_t operator()(const mav::ChunkCoordinates& coords) const {
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