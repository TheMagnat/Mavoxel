
#include <Mesh/DebugVoxel.hpp>

#include <Core/Global.hpp>

namespace mav {

	DebugVoxel::DebugVoxel(Shader* shaderPtr, Environment* environment, Material material, float size, glm::vec3 position)
        : Mesh(3, {{3}}, shaderPtr, environment, material, size, position), color_(1.0f, 1.0f, 1.0f) {}

    void DebugVoxel::generateVertices() {

        size_t nbOfData = 3;

        size_t verticesNb = 6*4*nbOfData; //Number of face * number of vertices per face * number of information per vertice
        indicesSize_ = 6*4*2; //Number of face * number of line * number of vertice per line

        vertices_.resize(verticesNb);
        indices_.resize(indicesSize_);

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

    void DebugVoxel::setColor(glm::vec3 const& color) {
        color_ = color;
    }

    void DebugVoxel::draw() const {

		glBindVertexArray(vao_.get());
		
		//SET LE SHADER
		shader_->use();
    
		glm::mat4 model = glm::mat4(1.0f);
        model = model * translationMatrix_;
        //model = glm::rotate(model, 45.0f, glm::vec3(0.0f, 1.0f, 1.0f));
		//model = model * rotationMat_;
		model = glm::scale(model, glm::vec3(size));


		shader_->setMat4("model", model);

		////TOUT LES NEED
		shader_->use();

		shader_->setVec3("Color", color_);

		//Calcule camera
		glm::mat4 view(environment_->camera->GetViewMatrix());

		shader_->setMat4("view", view);
		shader_->setMat4("projection", environment_->camera->Projection);


		glDrawElements(GL_LINES, (int)indices_.size(), GL_UNSIGNED_INT, 0);

	}

}