#pragma once

#include <GLObject/Shader.hpp>

#include <vector>
#include <cstddef>
#include <Helper/ThreadPool.hpp>

namespace mav {

	class Global{

		public:

			static size_t width;
			static size_t height;

		#ifndef NDEBUG

			static mav::Shader debugShader;

		#endif
		
	};



}