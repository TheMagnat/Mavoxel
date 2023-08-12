
#include <Files/FileHandler.hpp>

#include <filesystem>

namespace mav {

    void saveFile(std::string const& content, std::string const& file, std::string const& path) {

        std::ofstream ofs(RESSOURCES_PATH + path + "/" + file, std::ios::out);
        if (!ofs) throw std::invalid_argument("Impossible to save " + std::string(RESSOURCES_PATH) + path + "/" + file + ".");

        ofs << content;

    }

    std::string openFile(std::string const& file, std::string const& path) {

        std::ifstream in(RESSOURCES_PATH + path + "/" + file, std::ios::in);
        if (!in) throw std::invalid_argument("Impossible to read " + std::string(RESSOURCES_PATH) + path + "/" + file + ".");

        std::string contents;

        //To compute the full size for the string
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        
        //To read the whole data on the just allocated string
        in.seekg(0, std::ios::beg);
        //TODO: contents.data();
        in.read(&contents[0], contents.size());

        return contents;
        
    }

    std::ifstream getReadFileStream(std::string const& file, std::string const& path) {
        std::ifstream in(RESSOURCES_PATH + path + "/" + file, std::ios::in | std::ios::binary);
        if (!in) throw std::invalid_argument("Impossible to read " + std::string(RESSOURCES_PATH) + path + "/" + file + ".");

        return in;
    }

    std::ofstream getWriteFileStream(std::string const& file, std::string const& path) {

        std::filesystem::path fullPath = RESSOURCES_PATH + path;
        fullPath /= file;

        if (!std::filesystem::exists(fullPath.parent_path())) {
            if (!std::filesystem::create_directories(fullPath.parent_path())) {
                throw std::invalid_argument("Impossible to create " + std::string(RESSOURCES_PATH) + path + " directory.");
            }
        }

        std::ofstream out(fullPath, std::ios::out | std::ios::binary);
        if (!out) throw std::invalid_argument("Impossible to write " + fullPath.string() + ".");

        return out;
    }

}
