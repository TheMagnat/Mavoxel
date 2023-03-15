
#include <Mesh/DebugVoxel.hpp>

#include <Core/Global.hpp>

namespace mav {

	DebugVoxel::DebugVoxel(Shader* shaderPtr, Environment* environment, Material material, float size, glm::vec3 position)
        : Mesh(shaderPtr, environment, material, size, position) {}

	void DebugVoxel::init(){
		vao_.init(true);
        generateVertices();
        updateVAO();
	}

    void DebugVoxel::generateVertices(){

        size_t nbOfData = 9;

        size_t verticesNb = 6*4*nbOfData; //Number of face * number of vertices per face * number of information per vertice
        indicesNb_ = 6*4*2; //Number of face * number of line * number of vertice per line

        vertices_.resize(verticesNb);
        indices_.resize(indicesNb_);

        glm::vec3 color(0, 1, 0);
        float verticeLenght = 0.5f;


        std::vector<std::pair<size_t, float>> faceFixedValue {
            {1, -1}, //bottom
            {2, 1}, //front
            {0, 1}, //right
            {2, -1}, //back
            {0, -1}, //left
            {1, 1} //top
        };

        /**
         * This vector store a pair for each face that indicate the order at which each axe need to move.
         * The first element in the pair indicate the indexes of the axes and the second element indicate
         * the first value of each axe.
        */
        std::vector<std::pair<std::vector<size_t>, std::vector<float>>> faceAlternateValues {
            {{0, 2}, {-1, 1}}, //bottom
            {{0, 1}, {-1, 1}}, //front
            {{2, 1}, {1, 1}}, //right
            {{0, 1}, {1, 1}}, //back
            {{2, 1}, {-1, 1}}, //left
            {{0, 2}, {-1, -1}} //top
        };
        
        for (size_t i = 0; i < 6; ++i)
        {

            //We calculate the vertices
            size_t fixedIndex = faceFixedValue[i].first;
            float fixedValue = faceFixedValue[i].second;

            size_t secondIndex = faceAlternateValues[i].first[0];
            size_t firstIndex  = faceAlternateValues[i].first[1];

            float secondValue = faceAlternateValues[i].second[0];
            float firstValue = faceAlternateValues[i].second[1];

            //Indice start for each face (face index * number of vertice per face * number of information per vertice)
            size_t faceOffset = i*4*nbOfData;

            for(size_t j = 0; j < 4; ++j){

                //We also add (vertice index * nbOfData) to get current vertice position
                size_t verticeOffset = j*nbOfData;

                //Position
                vertices_[faceOffset + verticeOffset + fixedIndex] = fixedValue * verticeLenght;
                vertices_[faceOffset + verticeOffset + secondIndex] = secondValue * verticeLenght;
                vertices_[faceOffset + verticeOffset + firstIndex] = firstValue * verticeLenght;

                //Normals
                vertices_[faceOffset + verticeOffset + 3 + 0] = 0;
                vertices_[faceOffset + verticeOffset + 3 + 1] = 0;
                vertices_[faceOffset + verticeOffset + 3 + 2] = 0;
                vertices_[faceOffset + verticeOffset + 3 + fixedIndex] = fixedValue;

                //Color
                vertices_[faceOffset + verticeOffset + 6 + 0] = color.r;
                vertices_[faceOffset + verticeOffset + 6 + 1] = color.g;
                vertices_[faceOffset + verticeOffset + 6 + 2] = color.b;

                //Here we change first value
                if (j % 2 == 0) {
                    firstValue *= -1;
                }

                if (j == 1) {
                    secondValue *= -1;
                }

            }

            //We calculate the indices

            //Each face have 6 indices
            //First triangle
            indices_[i*8 + 0] = i*4 + 0;
            indices_[i*8 + 1] = i*4 + 1;

            indices_[i*8 + 2] = i*4 + 1;
            indices_[i*8 + 3] = i*4 + 2;

            indices_[i*8 + 4] = i*4 + 2;
            indices_[i*8 + 5] = i*4 + 3;

            indices_[i*8 + 6] = i*4 + 3;
            indices_[i*8 + 7] = i*4 + 0;

        }

    }

    void DebugVoxel::updateVAO(){

        std::vector<VAO::Attribute> allAttribute = {{3}, {3}, {3}};

		vao_.setAll(vertices_, 9, allAttribute, indices_);
    }

    void DebugVoxel::draw(){

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


		glDrawElements(GL_LINES, (int)indices_.size(), GL_UNSIGNED_INT, 0);

	}

}