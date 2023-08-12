#include <Mesh/Mesh.hpp>
#include <Core/Global.hpp>

#include <glm/gtx/transform.hpp>

namespace mav {

    Mesh::Mesh(size_t attributesSum, Environment* environmentP, Material materialP, float sizeP, glm::vec3 positionP)
        : Drawable(), size(sizeP), position(positionP), material(materialP), environment_(environmentP) {
            updatePosition();
        }

    void Mesh::setPosition(glm::vec3 const& newPosition){
        position = newPosition;
        updatePosition();
    }

    void Mesh::setPosition(float x, float y, float z){
        position.x = x;
        position.y = y;
        position.z = z;
        updatePosition();
    }

    void Mesh::updatePosition(){
        translationMatrix_ = glm::translate(position);
    }

    glm::vec3 const& Mesh::getPosition() const{
        return position;
    }

}