
#include "Core/Global.hpp"


namespace mav {
	
	size_t Global::width = 0;
	size_t Global::height = 0;

	ThreadPool Global::threadPool(4);


	#ifndef NDEBUG

		Shader Global::debugShader = Shader();

	#endif

}