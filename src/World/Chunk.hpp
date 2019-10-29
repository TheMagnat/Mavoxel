
#pragma once

#include <vector>

namespace mav{
	
	class Chunk{

		public:
			Chunk(size_t size);

			void describe();
			
		private:
			size_t size_;
	};

}