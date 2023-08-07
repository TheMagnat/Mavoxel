
#pragma once

#include <World/CoordinatesHelper.hpp>

#include <glm/glm.hpp>

#include <unordered_map>
#include <mutex>


namespace mav {
    
    class Chunk;
    class ChunkIndexer {

        public:

            ChunkIndexer(std::string const& worldFolderPath);

            void addChunk(glm::ivec3 const& position);

            size_t getChunk(glm::ivec3 const& position);

            //TODO: prendre "std::unordered_map<glm::ivec3, Chunk> const& chunks" en argument et les sauvegarder en même temps
			void save(std::unordered_map<glm::ivec3, Chunk> const& chunks) const;

            void load();


        private:

            std::string worldFolderPath_;

            //Note: index 0 is reserved as a return code
            size_t currentIndex_ = 1;
            std::unordered_map<glm::ivec3, size_t> posToIndex_;

            //Thread safe
            std::mutex safeAddMutex;

    };

}
