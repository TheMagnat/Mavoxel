
#include <Generator/VoxelMapGenerator.hpp>


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

bool ClassicVoxelMapGenerator::isIn(float x, float y, float z) const {
    
    // auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    // float testValue = fnFractal->GenSingle2D(1.f, 2.f, 10);

    // auto gen = FastNoise::New<FastNoise::Perlin>();

    // float testValue = gen->GenSingle2D(x, z, seed);
    // float testValue_2 = gen->GenSingle2D(x/100.0, z/100.0, seed);
    // float testValue_3 = gen->GenSingle2D(x/100.0, z/100.0, seed) * 32;

    // auto fnSimplex = FastNoise::New<FastNoise::Simplex>();


    

    return fnFractal_->GenSingle3D(x/factor_, y/factor_, z/factor_, seed_) >= 0.0f;
}


//TODO: Vérifier les perfs entre calculer la matrice puis regarder au dessus ou directement faire un appel a la noise fonction pour regarder au dessus
mav::VoxelData ClassicVoxelMapGenerator::generate(float xGlobal, float yGlobal, float zGlobal) const {

    mav::VoxelData output;
    output.count = 0;

    float positionOffsets = - ((chunkSize_ - 1) / 2.0f) * voxelSize_;

    output.map.resize(chunkSize_);
    for (size_t x = 0; x < chunkSize_; ++x) {

        output.map[x].resize(chunkSize_);
        for (int y = chunkSize_ - 1; y >= 0; --y) {

            output.map[x][y].resize(chunkSize_);
            for (size_t z = 0; z < chunkSize_; ++z) {

                float xPos = (x * voxelSize_) + positionOffsets + (xGlobal * chunkSize_ * voxelSize_);
                float yPos = (y * voxelSize_) + positionOffsets + (yGlobal * chunkSize_ * voxelSize_);
                float zPos = (z * voxelSize_) + positionOffsets + (zGlobal * chunkSize_ * voxelSize_);

                bool isVoxelIn = isIn(xPos, yPos, zPos);

                if (isVoxelIn) {

                    ++output.count;

                    
                    //TODO: Gérer le cas du début
                    if (y == chunkSize_ - 1) {
                        
                        size_t countUpperVoxel = 0;
                        float upperYPos = yPos;
                        while(countUpperVoxel <= 5) {
                            upperYPos += voxelSize_;
                            bool isUpperVoxelIn = isIn(xPos, upperYPos, zPos);

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

    //Corect last y level
    for (size_t x = 0; x < chunkSize_; ++x) {
        for (size_t z = 0; z < chunkSize_; ++z) {
            if (output.map[x][0][z] > 5) output.map[x][0][z] = 3;
            else if (output.map[x][0][z] > 1) output.map[x][0][z] = 2;
            else if (output.map[x][0][z] > 0) output.map[x][0][z] = 1;
        }
    }

    return output;

}