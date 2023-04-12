
#include "Core/Global.hpp"


namespace mav {
	
	size_t Global::width = 0;
	size_t Global::height = 0;

	#ifndef NDEBUG

		Shader Global::debugShader = Shader();
		Environment Global::debugEnvironment = {0, 0};

	#endif

}