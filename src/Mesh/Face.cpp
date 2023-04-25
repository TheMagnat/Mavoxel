
#include <Mesh/Face.hpp>

#include <Core/Global.hpp>


namespace mav {

	Face::Face(Shader* shaderPtr, Environment* environment, Material material, float size, glm::vec3 position)
        : Mesh(8, {{3}, {3}, {2}}, shaderPtr, environment, material, size, position) {}

    void Face::generateVertices() {}

    void Face::generateVertices(std::array<glm::vec3, 4> const& points){

        size_t nbOfData = 8;

        size_t verticesNb = 4*nbOfData; //Number of face * number of vertices per face * number of information per vertice
        indicesSize_ = 2*3; //Number of triangle * number of point per triangle

        vertices_.resize(verticesNb);
        indices_.resize(indicesSize_);

        glm::vec3 color(0, 1, 0);

        static const std::vector<std::pair<float, float>> verticesTexturesPositions {
            {0, 1},
            {0, 0},
            {1, 0},
            {1, 1}
        };

        for(size_t j = 0; j < 4; ++j){

            //We also add (vertice index * nbOfData) to get current vertice position
            size_t verticeOffset = j*nbOfData;

            //Position
            vertices_[verticeOffset + 0 + 0] = points[j].x;
            vertices_[verticeOffset + 0 + 1] = points[j].y;
            vertices_[verticeOffset + 0 + 2] = points[j].z;

            //Normals
            vertices_[verticeOffset + 3 + 0] = 0;
            vertices_[verticeOffset + 3 + 1] = 0;
            vertices_[verticeOffset + 3 + 2] = 0;
            //vertices_[verticeOffset + 3 + fixedIndex] = fixedValue;

            //Color
            vertices_[verticeOffset + 6 + 0] = verticesTexturesPositions[j].first;
            vertices_[verticeOffset + 6 + 1] = verticesTexturesPositions[j].second;

        }

        //We calculate the indices
        //First triangle
        indices_[0] = 0;
        indices_[1] = 1;
        indices_[2] = 3;
        
        //Second triangle
        indices_[3] = 2;
        indices_[4] = 3;
        indices_[5] = 1;

    }

    void Face::draw(){

		glBindVertexArray(vao_.get());
		

		//SET LE SHADER
		shader_->use();
    
		glm::mat4 model = glm::mat4(1.0f);
        model = model * translationMatrix_;
        //model = glm::rotate(model, 45.0f, glm::vec3(0.0f, 1.0f, 1.0f));
		//model = model * rotationMat_;
		//model = glm::scale(model, glm::vec3(size));


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

        shader_->setFloat("time", environment_->totalElapsedTime);

		//Calcule camera

		glm::mat4 view(environment_->camera->GetViewMatrix());
		//glm::mat4 view(glm::lookAt(cameraPtr_->Position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

		shader_->setMat4("view", view);

		//Position de la cam
		shader_->setVec3("viewPos", environment_->camera->Position);

		shader_->setMat4("projection", environment_->camera->Projection);


		glDrawElements(GL_TRIANGLES, (int)indices_.size(), GL_UNSIGNED_INT, 0);

	}

}
