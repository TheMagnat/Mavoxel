
#include "World/Chunk.hpp"

#include <iostream>

namespace mav{
	
	Chunk::Chunk(size_t size) : size_(size){

	}

	void Chunk::describe(){
		std::cout << "Je suis un chunk de taille : " << size_ << std::endl;
	}

}