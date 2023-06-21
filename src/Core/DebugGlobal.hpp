#pragma once

#include <VulkanWrapper/MultiShader.hpp>
#include <Environment/Environment.hpp>

#include <memory>


namespace mav {

	class DebugGlobal{

		public:
			
			static std::unique_ptr<vuw::MultiShader> debugShader;
			static mav::Environment debugEnvironment;

		
	};

}