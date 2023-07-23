
#pragma once

#include <glm/vec3.hpp>

#include <vector>

namespace mav {
    
    using VoxelMap = std::vector<std::vector<std::vector<int>>>;
    struct VoxelData {
        VoxelMap map;
        size_t count;
    };

    class VoxelMapGenerator {

        public:
            virtual bool isIn(glm::ivec3 const& position) const = 0;
            virtual VoxelData generate(int xGlobal, int yGlobal, int zGlobal) const = 0;

    };

}
