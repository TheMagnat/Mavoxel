
#include <Mesh/LightVoxel.hpp>

#include <Core/Global.hpp>
#include <GLObject/BufferTemplates.hpp>

namespace mav {

    LightVoxel::LightVoxel(Environment* environment, Material material, size_t size)
        : Voxel(environment, material, size) {}

    void LightVoxel::updateUniforms(vuw::Shader* shader, uint32_t currentFrame) const {
        //Binding 0
        ModelViewProjectionObject mvp{};
        glm::mat4 model(1.0f);
        model = glm::translate(model, environment_->sun->getPosition());
        model = glm::scale(model, glm::vec3(size));

        mvp.model = model;
        mvp.view = environment_->camera->GetViewMatrix();
        mvp.projection = environment_->camera->Projection;
        mvp.projection[1][1] *= -1;

        shader->updateUniform(0, currentFrame, &mvp, sizeof(mvp));
    }

}
