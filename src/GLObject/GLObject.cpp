
#include "GLObject/GLObject.hpp"
#include <iostream>
namespace mav {

	VAO::VAO(){}


	VAO::VAO(std::vector<float> const& vertices, unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute){
		init(false);
		setAll(vertices, totalAttributeSize, allAttribute);
	}

	VAO::VAO(std::vector<float> const& vertices, unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute, std::vector<int> const& indice){
		init(true);
		setAll(vertices, totalAttributeSize, allAttribute, indice);
	}

	void VAO::init(bool isEBO){
		glGenVertexArrays(1, &VAO_);
		glGenBuffers(1, &VBO_);

		if(isEBO){
			glGenBuffers(1, &EBO_);
		}
	}

	//WITHOUT EBO
	void VAO::setAll(std::vector<float> const& vertices, unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute){
		
		glBindVertexArray(VAO_);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_);


		setData(vertices);
		setAttribute(totalAttributeSize, allAttribute);

		glBindVertexArray(0);

	}

	//WITH EBO
	void VAO::setAll(std::vector<float> const& vertices, unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute, std::vector<int> const& indice){
		
		glBindVertexArray(VAO_);
		//glBindBuffer(GL_ARRAY_BUFFER, VBO_);

		setIndice(indice);
		setData(vertices);
		setAttribute(totalAttributeSize, allAttribute);

		glBindVertexArray(0);

	}

	void VAO::setData(std::vector<float> const& vertices){
		
		glBindBuffer(GL_ARRAY_BUFFER, VBO_);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	}


	void VAO::setAttribute(unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute){
		
		for(size_t i(0), acc(0); i < allAttribute.size(); acc += allAttribute[i].size, ++i){

			glVertexAttribPointer(i, allAttribute[i].size, GL_FLOAT, GL_FALSE, totalAttributeSize * sizeof(float), (void*)(acc*sizeof(float)));
			glEnableVertexAttribArray(i);

		}

	}

	void VAO::setIndice(std::vector<int> const& indice){
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice.size() * sizeof(int), indice.data(), GL_STATIC_DRAW);
	}

	unsigned int VAO::get() const {
		return VAO_;
	}
















}