
#pragma once

// #include <glad/glad.h>


namespace mav {

    class Texture3D {
        
        public:
            Texture3D (size_t size) : size_(size) {
            
                // glGenTextures(1, &id_);

                // //TODO: le faire ici ?
                // glBindTexture(GL_TEXTURE_3D, id_);

                // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
                // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            }

            void setData(std::vector<uint8_t> const& dataMatrix) {
                // glBindTexture(GL_TEXTURE_3D, id_);

                // glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, size_, size_, size_, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, dataMatrix.data());
            }

            void bind(unsigned int textureUnit = 0) const {
                // glActiveTexture(GL_TEXTURE0 + textureUnit);
                // glBindTexture(GL_TEXTURE_3D, id_);
            }

        private:
            unsigned int id_;

            size_t size_;

    };

}