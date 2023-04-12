
#include <World/Generator.hpp>

#include <FastNoise/FastNoise.h>


#include <vector>


class ClassicVoxelMapGenerator : public mav::VoxelMapGenerator {

    public:

        ClassicVoxelMapGenerator(size_t seed, size_t chunkSize, float voxelSize);

        bool isIn(float x, float y, float z) const;
        mav::VoxelData generate(float xGlobal, float yGlobal, float zGlobal) const;

    private:
        size_t seed_;

        float voxelSize_;
        size_t chunkSize_;

        // Noise parameters
        float factor_;

        FastNoise::SmartNode<FastNoise::Perlin> perlinGenerator_;
        FastNoise::SmartNode<FastNoise::Simplex> simplexGenerator_;
        FastNoise::SmartNode<FastNoise::FractalFBm> fnFractal_;
};
