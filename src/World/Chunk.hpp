
#pragma once

#include <vector>
#include <cstddef>

namespace mav{
	
	class Chunk{

		public:
			Chunk(size_t size);

			void describe();
			
		private:
			size_t size_;
	};

}