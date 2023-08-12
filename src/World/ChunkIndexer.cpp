
#include <World/ChunkIndexer.hpp>

#include <World/Chunk.hpp>

#include <files/DataFileConverter.hpp>
#include <files/FileHandler.hpp>


//TODO: set in CPP file
#define INDEX_FILE_NAME "index.mvx"

namespace mav {

    ChunkIndexer::ChunkIndexer(std::string const& worldFolderPath) : worldFolderPath_(worldFolderPath) {}

    void ChunkIndexer::addChunk(glm::ivec3 const& position) {
        std::lock_guard<std::mutex> lock(safeAddMutex);
        posToIndex_.emplace(position, currentIndex_++);
    }

    size_t ChunkIndexer::getChunk(glm::ivec3 const& position) {
        auto it = posToIndex_.find(position);
        if (it == posToIndex_.end()) return 0;
        return it->second;
    }

    //TODO: prendre "std::unordered_map<glm::ivec3, Chunk> const& chunks" en argument et les sauvegarder en même temps
    void ChunkIndexer::save(std::unordered_map<glm::ivec3, Chunk> const& chunks) const {

        //Load writing stream of index file
        std::ofstream stream = mav::getWriteFileStream(INDEX_FILE_NAME, worldFolderPath_);

        size_t dataSize = posToIndex_.size();
        stream.write(reinterpret_cast<const char*>(&dataSize), sizeof(size_t));

        for (std::pair<glm::ivec3, size_t> const& element : posToIndex_) {

            DataFileConverter::convertIntoStream(stream, {
                {DataFileConverter::DataType::IVEC3, (void*)&element.first},
                {DataFileConverter::DataType::SIZE_T, (void*)&element.second}
            });

            auto chunksIt = chunks.find(element.first);
            //This should never happen
            if (chunksIt == chunks.end()){
                //TODO: Faire un truc safe AU CAS ou ça arrive
                throw std::exception("A chunk present in the chunk indexer is not present in the given map in the save method, this should be impossible.");
            }

            //Load writing stream of chunk file
            std::ofstream stream = mav::getWriteFileStream(std::to_string(element.second) + ".mvx", worldFolderPath_);
            mav::DataFileConverter::convertIntoStream(stream, mav::DataFileConverter::DataFileDescription{
                {DataFileConverter::DataType::SVO, (void*) &(chunksIt->second.svo_)}
            });


        }

    }

    void ChunkIndexer::load() {
        
        //Load reading stream of index file
        std::ifstream stream = mav::getReadFileStream(INDEX_FILE_NAME, worldFolderPath_);

        size_t dataSize;
        stream.read(reinterpret_cast<char*>(&dataSize), sizeof(size_t));

        for (size_t i = 0; i < dataSize; ++i) {
            
            glm::ivec3 chunkPosition;
            size_t chunkIndex;

            DataFileConverter::convertFromStream(stream, {
                {DataFileConverter::DataType::IVEC3, &chunkPosition},
                {DataFileConverter::DataType::SIZE_T, &chunkIndex}
            });

            posToIndex_.emplace(chunkPosition, chunkIndex);

        }

    }

}
