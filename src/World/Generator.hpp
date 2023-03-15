
#pragma once


#include <functional>

namespace mav {

    //Terrain generator func
    using VoxelGeneratorFunc = std::function<int(float x, float y, float z)>;
    using VoxelTestFunc = std::function<bool(float x, float y, float z)>;
    
    using VoxelMap = std::vector<std::vector<std::vector<int>>>;
    using VoxelMapGeneratorFunc = std::function<VoxelMap(float startX, float startY, float startZ)>;


    class VoxelMapGenerator {

        public:
            virtual bool isIn(float x, float y, float z) const = 0;
            virtual VoxelMap generate(float xGlobal, float yGlobal, float zGlobal) const = 0;

    };

}
