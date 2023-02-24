
#include "Mesh/Plane.hpp"

#include "Core/Global.hpp"

#include <array>


namespace mav {
	
	Plane::Plane(Shader* shaderPtr, Camera* cameraPtr, size_t size) : size_(size), sizeVec_(size_/2.0f, 1.0, size_/2.0f), rotationMat_(1.0f), shaderPtr_(shaderPtr), cameraPtr_(cameraPtr) {

	}

	void Plane::init(){
		vao_.init(true);
	}

	void Plane::set(size_t verticesNb, size_t len, std::vector<float> const& height){

		len_ = len;
		row_ = verticesNb/len_;

		vertices_.resize(verticesNb * 8);

		//SAVE
		indicesNb_ = 6 * (len_ - 1)*(row_ - 1);
		
		//SAVE
		indices_.resize(indicesNb_);

		//float middle(size_/2);


		size_t vertexPointer = 0;
		for (size_t i = 0; i < row_; ++i) {
		    for (size_t j = 0; j < len_; ++j) {
		        
		        float tempoX((float)j * 2 / ((float)len_ - 1) - 1);
		        float tempoZ((float)i * 2 / ((float)row_ - 1) - 1);
		        
		        
		        //float x(tempoX * middle), y(tempoZ * middle);
		        
		        //float verticeWorldX(x_ * size_ + x), verticeWorldZ(y_ * size_ + y);
		        
		        //float tempoHeight(heightGenerator_->generateHeight(verticeWorldX, verticeWorldZ));
		        float tempoHeight(height[i * len_ + j]);
		        
		        //Position
		        vertices_[vertexPointer * 8] = tempoX;
		        vertices_[vertexPointer * 8 + 1] = tempoHeight;
		        vertices_[vertexPointer * 8 + 2] = tempoZ;
		        
		        //Normals     
		        vertices_[vertexPointer * 8 + 3] = 0.0f;
		        vertices_[vertexPointer * 8 + 4] = 1.0f;
		        vertices_[vertexPointer * 8 + 5] = 0.0f;
		        
		        //Texture
		        vertices_[vertexPointer * 8 + 6] = (float)j / ((float)len_ - 1);
		        vertices_[vertexPointer * 8 + 7] = (float)i / ((float)row_ - 1);
		        
		        ++vertexPointer;
		    }
		}

		size_t pointer = 0;
		for (size_t gz = 0; gz < row_ - 1; ++gz) {
		    for (size_t gx = 0; gx < len_ - 1; ++gx) {
		        
		        int topLeft = (gz * len_) + gx;
		        int topRight = topLeft + 1;
		        int bottomLeft = ((gz + 1) * len_) + gx;
		        int bottomRight = bottomLeft + 1;
		        
		        indices_[pointer++] = topLeft;
		        indices_[pointer++] = bottomLeft;
		        indices_[pointer++] = topRight;
		        indices_[pointer++] = topRight;
		        indices_[pointer++] = bottomLeft;
		        indices_[pointer++] = bottomRight;
		        
		    }
		}

		//calculateNormals();
		//Fin methode set
	}


	//CALCUL NORMAL
	void Plane::calculateNormals() {
	    
	    size_t vertexPointer = 0;
	    for (size_t i = 0; i < row_; i++) {
	        for (size_t j = 0; j < len_; j++) {
	            
	            size_t p1((i - 1) * len_ + j), p2(i * len_ + j);
	            
	            std::array<int, 6> faceIndex;
	            std::array<glm::vec3, 6> faceNormal;
	            
	            faceIndex[0] = ((p1 - (i - 1)) * 2 - 1);
	            faceIndex[1] = (faceIndex[0] + 1);
	            faceIndex[2] = (faceIndex[1] + 1);
	            faceIndex[3] = ((p2 - i) * 2 - 2);
	            faceIndex[4] = (faceIndex[3] + 1);
	            faceIndex[5] = (faceIndex[4] + 1);
	            
	            
	            
	            
	            if (i == 0) {
	                faceIndex[0] = -1;
	                faceNormal[0] = calculateNormal(j, i - 1, j - 1, i, j, i);
	                
	                faceIndex[1] = -1;
	                faceNormal[1] = calculateNormal(j, i - 1, j, i, j + 1, i - 1);
	                
	                
	                faceIndex[2] = -1;
	                faceNormal[2] = calculateNormal(j + 1, i - 1, j, i, j + 1, i);
	                
	            }
	            else if (i == row_ - 1) {
	                faceIndex[3] = -1;
	                faceNormal[3] = calculateNormal(j - 1, i, j - 1, i + 1, j, i);
	                
	                faceIndex[4] = -1;
	                faceNormal[4] = calculateNormal(j, i, j - 1, i + 1, j, i + 1);
	                
	                faceIndex[5] = -1;
	                faceNormal[5] = calculateNormal(j, i, j, i + 1, j + 1, i);
	                
	            }
	            
	            if (j == 0) {
	                faceIndex[0] = -1;
	                faceNormal[0] = calculateNormal(j, i - 1, j - 1, i, j, i);
	                
	                faceIndex[3] = -1;
	                faceNormal[3] = calculateNormal(j - 1, i, j - 1, i + 1, j, i);
	                
	                faceIndex[4] = -1;
	                faceNormal[4] = calculateNormal(j, i, j - 1, i + 1, j, i + 1);
	                
	            }
	            else if (j == len_ - 1) {
	                faceIndex[1] = -1;
	                faceNormal[1] = calculateNormal(j, i - 1, j, i, j + 1, i - 1);
	                
	                faceIndex[2] = -1;
	                faceNormal[2] = calculateNormal(j + 1, i - 1, j, i, j + 1, i);
	                
	                faceIndex[5] = -1;
	                faceNormal[5] = calculateNormal(j, i, j, i + 1, j + 1, i);
	                
	            }
	            
	            for (size_t i2(0); i2 < 6; ++i2) {

	                if (faceIndex[i2] == -1) {
	                    continue;
	                }
	                else {
	                    faceNormal[i2] = calculateNormal(faceIndex[i2]);
	                }
	            }
	            
	            glm::vec3 normal(glm::normalize(faceNormal[0] + faceNormal[1] + faceNormal[2] + faceNormal[3] + faceNormal[4] + faceNormal[5]));
	            
	            vertices_[vertexPointer * 8 + 3] = normal.x;
	            vertices_[vertexPointer * 8 + 4] = normal.y;
	            vertices_[vertexPointer * 8 + 5] = normal.z;
	            
	            ++vertexPointer;
	        }
	    }
	    
	}

	glm::vec3 Plane::calculateNormal(int x1, int z1, int x2, int z2, int x3, int z3){
	    
	    //float middle(size_/2);
	    
	    float tempoX1((float)x1 * 2 / ((float)len_ - 1) - 1), tempoZ1((float)z1 * 2 / ((float)row_ - 1) - 1);
	    float tempoX2((float)x2 * 2 / ((float)len_ - 1) - 1), tempoZ2((float)z2 * 2 / ((float)row_ - 1) - 1);
	    float tempoX3((float)x3 * 2 / ((float)len_ - 1) - 1), tempoZ3((float)z3 * 2 / ((float)row_ - 1) - 1);
	    
	    // glm::vec3 p1(tempoX1, heightGenerator_->generateHeight(tempoX1 * middle + worldPosX_ * size_, tempoZ1 * middle + worldPosY_ * size_), tempoZ1);
	    // glm::vec3 p2(tempoX2, heightGenerator_->generateHeight(tempoX2 * middle + worldPosX_ * size_, tempoZ2 * middle + worldPosY_ * size_), tempoZ2);
	    // glm::vec3 p3(tempoX3, heightGenerator_->generateHeight(tempoX3 * middle + worldPosX_ * size_, tempoZ3 * middle + worldPosY_ * size_), tempoZ3);
	    
		glm::vec3 p1(tempoX1, 0.0, tempoZ1);
		glm::vec3 p2(tempoX2, 0.0, tempoZ2);
		glm::vec3 p3(tempoX3, 0.0, tempoZ3);


	    return glm::normalize(glm::cross(p2 - p1, p3 - p1));
	    
	}

	glm::vec3 Plane::calculateNormal(int face){
	    
	    face *= 3;
	    
	    
	    size_t a(indices_[face]), b(indices_[face+1]), c(indices_[face+2]);
	    
	    
	    float* tempo = &vertices_[a * 8];
	    glm::vec3 p1(vertices_[a * 8], vertices_[a * 8 + 1], vertices_[a * 8 + 2]);
	    
	    tempo = &vertices_[b * 8];
	    glm::vec3 p2(vertices_[b * 8], vertices_[b * 8 + 1], vertices_[b * 8 + 2]);
	    
	    tempo = &vertices_[c * 8];
	    glm::vec3 p3(vertices_[c * 8], vertices_[c * 8 + 1], vertices_[c * 8 + 2]);
	    
	    return glm::normalize(glm::cross(p2 - p1, p3 - p1));
	}




	void Plane::update(){

		std::vector<VAO::Attribute> allAttribute = {{3}, {3}, {2}};

		vao_.setAll(vertices_, 8, allAttribute, indices_);

	}
	
	void Plane::setPosition(float x, float y){
		x_ = x;
		y_ = y;
	}

	void Plane::setRotationMatrice(glm::mat4 const& rotaMat){
		rotationMat_ = rotaMat;
	}

	void Plane::setSizeVec(glm::vec3 const& sizeVec){

		sizeVec_ = sizeVec;

	}

	void Plane::draw(){

		glBindVertexArray(vao_.get());
		
		//SET LE SHADER
		shaderPtr_->use();
    
		glm::vec3 position(x_, 0, y_);
		glm::mat4 model(glm::translate(glm::mat4(1.0f), position));
		model = model * rotationMat_;
		model = glm::scale(model, sizeVec_);


		shaderPtr_->setMat4("model", model);
		shaderPtr_->setMat3("modelNormal", glm::mat3(glm::transpose(glm::inverse(model))));

		////TOUT LES NEED
		shaderPtr_->use();

		shaderPtr_->setVec3("material.diffuse", 0.3, 0.32, 0.29);
		shaderPtr_->setVec3("material.specular", 0.1, 0.1, 0.1);
		shaderPtr_->setFloat("material.shininess", 16.0f);


		shaderPtr_->setVec3("light.ambient",  0.3f, 0.3f, 0.3f);
		shaderPtr_->setVec3("light.diffuse",  1.f, 1.f, 1.f); // assombri un peu la lumière pour correspondre à la scène
		shaderPtr_->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		shaderPtr_->setVec3("light.direction", 0.1f, -0.8f, -0.2f);

		shaderPtr_->setFloat("light.constant",  1.0f);
		shaderPtr_->setFloat("light.linear",    0.09f);
		shaderPtr_->setFloat("light.quadratic", 0.032f);




		shaderPtr_->setVec3("light.position", glm::vec3(0, 50, 100));



		//Calcule camera

		glm::mat4 view(cameraPtr_->GetViewMatrix());
		//glm::mat4 view(glm::lookAt(cameraPtr_->Position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

		shaderPtr_->setMat4("view", view);

		//Position de la cam
		shaderPtr_->setVec3("viewPos", cameraPtr_->Position);

		shaderPtr_->setMat4("projection", glm::perspective(glm::radians(45.0f), (float)mav::Global::width / (float)mav::Global::height, 0.1f, 500.0f));


		glDrawElements(GL_TRIANGLES, (int)indicesNb_, GL_UNSIGNED_INT, 0);

	}


	// ColorPlane::ColorPlane(Shader* shaderPtr, Camera* cameraPtr, size_t size = 1){

	// }

	void ColorPlane::set(size_t verticesNb, size_t len, std::vector<float> const& height, std::vector<glm::vec3> const& colors){

		len_ = len;
		row_ = verticesNb/len_;

		vertices_.resize(verticesNb * 9);

		//SAVE
		indicesNb_ = 6 * (len_ - 1)*(row_ - 1);
		
		//SAVE
		indices_.resize(indicesNb_);

		//float middle(size_/2);


		size_t vertexPointer = 0;
		for (size_t i = 0; i < row_; ++i) {
		    for (size_t j = 0; j < len_; ++j) {
		        
		        float tempoX((float)j * 2 / ((float)len_ - 1) - 1);
		        float tempoZ((float)i * 2 / ((float)row_ - 1) - 1);
		        
		        
		        //float x(tempoX * middle), y(tempoZ * middle);
		        
		        //float verticeWorldX(x_ * size_ + x), verticeWorldZ(y_ * size_ + y);
		        
		        //float tempoHeight(heightGenerator_->generateHeight(verticeWorldX, verticeWorldZ));
		        float tempoHeight(height[i * len_ + j]);
				glm::vec3 tempoColor(colors[i * len_ + j]);
		        
		        //Position
		        vertices_[vertexPointer * 9] = tempoX;
		        vertices_[vertexPointer * 9 + 1] = tempoHeight;
		        vertices_[vertexPointer * 9 + 2] = tempoZ;
		        
		        //Normals     
		        vertices_[vertexPointer * 9 + 3] = 0.0f;
		        vertices_[vertexPointer * 9 + 4] = 1.0f;
		        vertices_[vertexPointer * 9 + 5] = 0.0f;
		        
				//Colors
				vertices_[vertexPointer * 9 + 6] = tempoColor[0];
		        vertices_[vertexPointer * 9 + 7] = tempoColor[1];
		        vertices_[vertexPointer * 9 + 8] = tempoColor[2];

		        
		        //Texture
		        // vertices_[vertexPointer * 8 + 6] = (float)j / ((float)len_ - 1);
		        // vertices_[vertexPointer * 8 + 7] = (float)i / ((float)row_ - 1);
		        
		        ++vertexPointer;
		    }
		}

		size_t pointer = 0;
		for (size_t gz = 0; gz < row_ - 1; ++gz) {
		    for (size_t gx = 0; gx < len_ - 1; ++gx) {
		        
		        int topLeft = (gz * len_) + gx;
		        int topRight = topLeft + 1;
		        int bottomLeft = ((gz + 1) * len_) + gx;
		        int bottomRight = bottomLeft + 1;
		        
		        indices_[pointer++] = topLeft;
		        indices_[pointer++] = bottomLeft;
		        indices_[pointer++] = topRight;
		        indices_[pointer++] = topRight;
		        indices_[pointer++] = bottomLeft;
		        indices_[pointer++] = bottomRight;
		        
		    }
		}

		//calculateNormals();
		//Fin methode set
	}


	void ColorPlane::update(){

		std::vector<VAO::Attribute> allAttribute = {{3}, {3}, {3}};

		vao_.setAll(vertices_, 9, allAttribute, indices_);

	}


}




