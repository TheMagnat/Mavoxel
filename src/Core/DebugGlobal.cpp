
#include <Core/DebugGlobal.hpp>


namespace mav {

	std::unique_ptr<vuw::MultiShader> DebugGlobal::debugShader = nullptr;
	Environment DebugGlobal::debugEnvironment = {0, 0};

}