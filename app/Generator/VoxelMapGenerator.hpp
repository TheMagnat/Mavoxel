
#pragma once

#include <World/Generator.hpp>

#include <FastNoise/FastNoise.h>

#include <glm/vec3.hpp>


#include <vector>


class ClassicVoxelMapGenerator : public mav::VoxelMapGenerator {

    public:

        ClassicVoxelMapGenerator(size_t seed, size_t chunkSize, float voxelSize);

        bool isIn(glm::ivec3 const& position) const;
        std::vector<float> batchIsIn(glm::ivec3 const& startPosition, size_t len) const;
        mav::VoxelData generate(glm::ivec3 const& globalPosition) const;

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
