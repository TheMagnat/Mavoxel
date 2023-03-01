#include <Mesh/Mesh.hpp>
#include <Core/Global.hpp>

#include <glm/gtx/transform.hpp>

namespace mav {

    Mesh::Mesh(Shader* shaderPtrP, Environment* environmentP, Material materialP, float sizeP, glm::vec3 positionP)
        : size(sizeP), position(positionP), material(materialP), environment_(environmentP), shaderPtr_(shaderPtrP) {
            updatePosition();
        }

    void Mesh::init(){
        vao_.init(true);
    }

    void Mesh::setPosition(glm::vec3 newPosition){
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

    glm::vec3 Mesh::getPosition(){
        return position;
    }

    void Mesh::draw(){

		glBindVertexArray(vao_.get());
		

		//SET LE SHADER
		shaderPtr_->use();
    
		glm::mat4 model = glm::mat4(1.0f);
        model = model * translationMatrix_;
        //model = glm::rotate(model, 45.0f, glm::vec3(0.0f, 1.0f, 1.0f));
		//model = model * rotationMat_;
		model = glm::scale(model, glm::vec3(size));


		shaderPtr_->setMat4("model", model);
		shaderPtr_->setMat3("modelNormal", glm::mat3(glm::transpose(glm::inverse(model))));

		////TOUT LES NEED
		shaderPtr_->use();

		shaderPtr_->setVec3("material.ambient", material.ambient);
		shaderPtr_->setVec3("material.diffuse", material.diffuse);
		shaderPtr_->setVec3("material.specular", material.specular);
		shaderPtr_->setFloat("material.shininess", material.shininess);

		shaderPtr_->setVec3("light.ambient",  environment_->sun->material.ambient);
		shaderPtr_->setVec3("light.diffuse",  environment_->sun->material.diffuse); // assombri un peu la lumière pour correspondre à la scène
		shaderPtr_->setVec3("light.specular", environment_->sun->material.specular);

		shaderPtr_->setVec3("light.position", environment_->sun->getPosition());

		// shaderPtr_->setFloat("light.constant",  1.0f);
		// shaderPtr_->setFloat("light.linear",    0.09f);
		// shaderPtr_->setFloat("light.quadratic", 0.032f);


		//shaderPtr_->setVec3("light.position", glm::vec3(0, 50, 100));



		//Calcule camera

		glm::mat4 view(environment_->camera->GetViewMatrix());
		//glm::mat4 view(glm::lookAt(cameraPtr_->Position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

		shaderPtr_->setMat4("view", view);

		//Position de la cam
		shaderPtr_->setVec3("viewPos", environment_->camera->Position);

		shaderPtr_->setMat4("projection", glm::perspective(glm::radians(45.0f), (float)mav::Global::width / (float)mav::Global::height, 0.1f, 2000.0f));


		glDrawElements(GL_TRIANGLES, (int)indicesNb_, GL_UNSIGNED_INT, 0);

	}

}