
#include <Generator/VoxelMapGenerator.hpp>

#ifdef TIME
//Help/debug
#include <Helper/Benchmark/Profiler.hpp>
#endif

ClassicVoxelMapGenerator::ClassicVoxelMapGenerator(size_t seed, size_t chunkSize, float voxelSize)
    : seed_(seed), chunkSize_(chunkSize), voxelSize_(voxelSize),
      perlinGenerator_(FastNoise::New<FastNoise::Perlin>()),
      simplexGenerator_(FastNoise::New<FastNoise::Simplex>()),
      fnFractal_(FastNoise::New<FastNoise::FractalFBm>())
{
    
    factor_ = 400.0f;

    fnFractal_->SetSource(perlinGenerator_);
    fnFractal_->SetOctaveCount(9);
}

bool ClassicVoxelMapGenerator::isIn(glm::ivec3 const& position) const {
    
    // auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    // float testValue = fnFractal->GenSingle2D(1.f, 2.f, 10);

    // auto gen = FastNoise::New<FastNoise::Perlin>();

    // float testValue = gen->GenSingle2D(x, z, seed);
    // float testValue_2 = gen->GenSingle2D(x/100.0, z/100.0, seed);
    // float testValue_3 = gen->GenSingle2D(x/100.0, z/100.0, seed) * 32;

    // auto fnSimplex = FastNoise::New<FastNoise::Simplex>();

    return fnFractal_->GenSingle3D(position.x * (voxelSize_ / factor_), position.y * (voxelSize_ / factor_), position.z * (voxelSize_ / factor_), seed_) >= 0.0f;
}

std::vector<float> ClassicVoxelMapGenerator::batchIsIn(glm::ivec3 const& startPosition, size_t len) const {

    #ifdef TIME
        Profiler profiler("Generate voxel map BATCH (VoxelMapGenerator)");
    #endif

    // fnFractal_->GenPositionArray3D(float* out, int count,
    //         const float* xPosArray, const float* yPosArray, const float* zPosArray, 
    //         float xOffset, float yOffset, float zOffset, int seed);

    std::vector<float> data(len*len*len);

    auto rez = fnFractal_->GenUniformGrid3D(data.data(),
            startPosition.x, startPosition.y, startPosition.z, 
            (int)len,  (int)len,  (int)len, 
            voxelSize_ / factor_, seed_);

    return data;
}


//TODO: Vérifier les perfs entre calculer la matrice puis regarder au dessus ou directement faire un appel a la noise fonction pour regarder au dessus
mav::VoxelData ClassicVoxelMapGenerator::generate(int xGlobal, int yGlobal, int zGlobal) const {
    
    #ifdef TIME
        Profiler profiler("Generate voxel map (VoxelMapGenerator)");
    #endif

    
    mav::VoxelData output;
    output.count = 0;

    glm::ivec3 startPosition(xGlobal * chunkSize_, yGlobal * chunkSize_, zGlobal * chunkSize_);

    std::vector<float> data = batchIsIn(startPosition, chunkSize_);

    output.map.resize(chunkSize_);
    for (size_t x = 0; x < chunkSize_; ++x) {

        output.map[x].resize(chunkSize_);
        for (int y = chunkSize_ - 1; y >= 0; --y) {

            output.map[x][y].resize(chunkSize_);
            for (size_t z = 0; z < chunkSize_; ++z) {

                bool isVoxelIn = data[x+ y * chunkSize_ + z * (chunkSize_ * chunkSize_)] >= 0.00f;
                if (isVoxelIn) {
            
                    ++output.count;

                    //TODO: Gérer le cas du début
                    if (y == chunkSize_ - 1) {
                        
                        size_t countUpperVoxel = 0;
                        float upperYPos = y;
                        while(countUpperVoxel <= 5) {
                            upperYPos += 1;
                            bool isUpperVoxelIn = isIn(startPosition + glm::ivec3(x, upperYPos, z));

                            if( !isUpperVoxelIn ) break;

                            ++countUpperVoxel;
                        }

                        output.map[x][y][z] = countUpperVoxel + 1;

                    }
                    else {
                        int upperVoxelId = output.map[x][y+1][z];
                        output.map[x][y][z] = upperVoxelId + 1;

                        if(upperVoxelId > 5) output.map[x][y+1][z] = 3;
                        else if(upperVoxelId > 1) output.map[x][y+1][z] = 2;
                        else if(upperVoxelId > 0) output.map[x][y+1][z] = 1;

                    }

                }
                else {
                    output.map[x][y][z] = 0;

                    //TODO: Améliorer la boucle entière pour la rendre plus propre/opti
                    if (y != chunkSize_ - 1) {
                        if(output.map[x][y+1][z] > 5) output.map[x][y+1][z] = 3;
                        else if(output.map[x][y+1][z] > 1) output.map[x][y+1][z] = 2;
                        else if(output.map[x][y+1][z] > 0) output.map[x][y+1][z] = 1;
                    }

                }

            }
        }

    }

    //Correct last y level
    for (size_t x = 0; x < chunkSize_; ++x) {
        for (size_t z = 0; z < chunkSize_; ++z) {
            if (output.map[x][0][z] > 5) output.map[x][0][z] = 3;
            else if (output.map[x][0][z] > 1) output.map[x][0][z] = 2;
            else if (output.map[x][0][z] > 0) output.map[x][0][z] = 1;
        }
    }
    
    return output;

}
