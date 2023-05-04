#include <Mesh/Mesh.hpp>
#include <Core/Global.hpp>

#include <glm/gtx/transform.hpp>

namespace mav {

    Mesh::Mesh(size_t attributesSum, std::vector<VAO::Attribute> const& attributes, Shader* shaderPtr, Environment* environmentP, Material materialP, float sizeP, glm::vec3 positionP)
        : Drawable(attributesSum, attributes, shaderPtr), size(sizeP), position(positionP), material(materialP), environment_(environmentP) {
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

    void Mesh::draw() const {

		glBindVertexArray(vao_.get());
		

		//SET LE SHADER
		shader_->use();
    
		glm::mat4 model = glm::mat4(1.0f);
        model = model * translationMatrix_;
        //model = glm::rotate(model, 45.0f, glm::vec3(0.0f, 1.0f, 1.0f));
		//model = model * rotationMat_;
		model = glm::scale(model, glm::vec3(size));


		shader_->setMat4("model", model);
		shader_->setMat3("modelNormal", glm::mat3(glm::transpose(glm::inverse(model))));

		////TOUT LES NEED
		shader_->use();

		shader_->setVec3("material.ambient", material.ambient);
		shader_->setVec3("material.diffuse", material.diffuse);
		shader_->setVec3("material.specular", material.specular);
		shader_->setFloat("material.shininess", material.shininess);

		shader_->setVec3("light.ambient",  environment_->sun->material.ambient);
		shader_->setVec3("light.diffuse",  environment_->sun->material.diffuse); // assombri un peu la lumière pour correspondre à la scène
		shader_->setVec3("light.specular", environment_->sun->material.specular);

		shader_->setVec3("light.position", environment_->sun->getPosition());

		// shader_->setFloat("light.constant",  1.0f);
		// shader_->setFloat("light.linear",    0.09f);
		// shader_->setFloat("light.quadratic", 0.032f);


		//shader_->setVec3("light.position", glm::vec3(0, 50, 100));



		//Calcule camera

		glm::mat4 view(environment_->camera->GetViewMatrix());
		//glm::mat4 view(glm::lookAt(cameraPtr_->Position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

		shader_->setMat4("view", view);

		//Position de la cam
		shader_->setVec3("viewPos", environment_->camera->Position);

		shader_->setMat4("projection", environment_->camera->Projection);


		glDrawElements(GL_TRIANGLES, (int)indicesSize_, GL_UNSIGNED_INT, 0);

	}

}