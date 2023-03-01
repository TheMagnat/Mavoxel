
#include <Mesh/LightVoxel.hpp>

#include <Core/Global.hpp>

namespace mav {

	LightVoxel::LightVoxel(Shader* shaderPtr, Environment* environment, Material material, size_t size)
        : Voxel(shaderPtr, environment, material, size) {}

}