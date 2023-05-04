#pragma once

#include <vector>
#include <fstream>


class ShaderHelper {

    public:

        static std::vector<char> readFile(const std::string& filename) {

            //Open at the end to get the length of the file
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open()) {
                throw std::runtime_error(std::string {"failed to open file "} + filename + " !");
            }

            //Initialize buffer with the good size
            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);

            //Fille the buffer with the whole file data
            file.seekg(0);
            file.read(buffer.data(), fileSize);

            return buffer;
        }


        static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create the shader module !");
            }

            return shaderModule;

        }

};
