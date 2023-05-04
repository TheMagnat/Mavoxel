#pragma once

#include <vector>

#include <glad/glad.h>


namespace mav {


	class VAO {
		
		public:

			struct Attribute{
				unsigned int size;
			};


			VAO();
			//ADD DESTRUCTOR TO RELEASE DATA
			/**
			*	Constructor that call setAll.
			*/
			VAO(std::vector<float> const& vertices, unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute);
			
			//Same as above but with an EBO.
			VAO(std::vector<float> const& vertices, unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute, std::vector<int> const& indice);

			operator bool() const;

			/**
			*	Configure the vertex Array.
			*	Vertices informations are stored in vertices.
			*	Attribute information are stored in allAttribute.
			*/
			void init(bool isEBO);

			void setAll(std::vector<float> const& vertices, unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute);
			//Same as above but with an EBO.
			void setAll(std::vector<float> const& vertices, unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute, std::vector<int> const& indice);


			void setData(std::vector<float> const& vertices);
			void setAttribute(unsigned int totalAttributeSize, std::vector<Attribute> const& allAttribute);
			void setIndices(std::vector<int> const& indices);

			unsigned int get() const;
			void bind() const;

		private:

			unsigned int VAO_ = 0;
			unsigned int VBO_ = 0;

			//If EBO activated
			unsigned int EBO_ = 0;

	};






}
