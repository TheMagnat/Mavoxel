
#include <World/CoordinatesHelper.hpp>

namespace mav {

        
        ChunkCoordinates::ChunkCoordinates(int xPos, int yPos, int zPos) : x(xPos), y(yPos), z(zPos) {} 

        bool ChunkCoordinates::operator==(ChunkCoordinates const& toCompare) const { 
            return (x == toCompare.x && y == toCompare.y && z == toCompare.z);
        }

}
