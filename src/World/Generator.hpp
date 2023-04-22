
#pragma once


#include <functional>
#include <vector>

namespace mav {

    //Terrain generator func
    using VoxelGeneratorFunc = std::function<int(float x, float y, float z)>;
    using VoxelTestFunc = std::function<bool(float x, float y, float z)>;
    
    using VoxelMap = std::vector<std::vector<std::vector<int>>>;
    struct VoxelData {
        VoxelMap map;
        size_t count;
    };

    class VoxelMapGenerator {

        public:
            virtual bool isIn(float x, float y, float z) const = 0;
            virtual VoxelData generate(float xGlobal, float yGlobal, float zGlobal) const = 0;

    };

}
