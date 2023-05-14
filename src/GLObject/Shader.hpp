#pragma once

// #include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace mav {

class Shader
{
public:
    // the program ID
    unsigned int ID;

    bool open_;
  
    // le constructeur lit et construit le shader
    Shader();
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

    void load(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    // Activation du shader
    void use() const;

    // fonctions utiles pour l'uniform
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;
    void setIntV(const std::string &name, std::vector<int> const& values);
    void setFloat(const std::string &name, float value) const;
    void set4Float(const std::string &name, float value1, float value2, float value3, float value4) const;

    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;

    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;

    void setIVec3(const std::string &name, const glm::ivec3 &value) const;

    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;

    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type);

};

}
