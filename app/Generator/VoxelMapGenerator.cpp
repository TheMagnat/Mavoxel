
#include <Generator/VoxelMapGenerator.hpp>


ClassicVoxelMapGenerator::ClassicVoxelMapGenerator(size_t seed, size_t chunkSize, float voxelSize)
    : seed_(seed), chunkSize_(chunkSize), voxelSize_(voxelSize), perlinGenerator_(FastNoise::New<FastNoise::Perlin>()), fnFractal_(FastNoise::New<FastNoise::FractalFBm>()) {
    
    factor_ = 800.0f;

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
mav::VoxelMap ClassicVoxelMapGenerator::generate(float xGlobal, float yGlobal, float zGlobal) const {

    mav::VoxelMap output;


    float positionOffsets = - ((chunkSize_) / 2.0f) * voxelSize_;

    output.resize(chunkSize_);
    for (size_t x = 0; x < chunkSize_; ++x) {

        output[x].resize(chunkSize_);
        for (int y = chunkSize_ - 1; y >= 0; --y) {

            output[x][y].resize(chunkSize_);
            for (size_t z = 0; z < chunkSize_; ++z) {

                float xPos = (x * voxelSize_) + positionOffsets + (xGlobal * chunkSize_ * voxelSize_);
                float yPos = (y * voxelSize_) + positionOffsets + (yGlobal * chunkSize_ * voxelSize_);
                float zPos = (z * voxelSize_) + positionOffsets + (zGlobal * chunkSize_ * voxelSize_);

                bool isVoxelIn = isIn(xPos, yPos, zPos);

                if (isVoxelIn) {
                    
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

                        output[x][y][z] = countUpperVoxel + 1;


                    }
                    else {
                        int upperVoxelId = output[x][y+1][z];
                        output[x][y][z] = upperVoxelId + 1;

                        if(upperVoxelId > 5) output[x][y+1][z] = 3;
                        else if(upperVoxelId > 2) output[x][y+1][z] = 2;
                    }

                }
                else {
                    output[x][y][z] = 0;

                    //TODO: Améliorer la boucle entière pour la rendre plus propre/opti
                    if (y != chunkSize_ - 1) {
                        if(output[x][y+1][z] > 5) output[x][y+1][z] = 3;
                        else if(output[x][y+1][z] > 2) output[x][y+1][z] = 2;
                    }

                }

            }
        }
    }

    //Corect last y level
    for (size_t x = 0; x < chunkSize_; ++x) {
        for (size_t z = 0; z < chunkSize_; ++z) {
            if (output[x][0][z] > 5) output[x][0][z] = 3;
            else if (output[x][0][z] > 2) output[x][0][z] = 2;
        }
    }

    return output;

}
