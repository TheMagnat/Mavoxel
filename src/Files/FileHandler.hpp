
#pragma once

#include <string>
#include <fstream>

#define RESSOURCES_PATH "data/"
#define DEFAULT_PATH ""

namespace mav {

    void saveFile(std::string const& content, std::string const& file, std::string const& path = DEFAULT_PATH);

    std::string openFile(std::string const& file, std::string const& path = DEFAULT_PATH);

    std::ifstream getReadFileStream(std::string const& file, std::string const& path = DEFAULT_PATH);

    std::ofstream getWriteFileStream(std::string const& file, std::string const& path = DEFAULT_PATH);

}
