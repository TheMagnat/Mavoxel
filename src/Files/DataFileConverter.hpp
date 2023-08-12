
#pragma once

#include <Octree/SparseVoxelOctree.hpp>

#include <glm/vec3.hpp>

#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>


namespace mav {

    class DataFileConverter {

        public:
            
            enum class DataType {
                VEC3, IVEC3, SVO, INT, FLOAT, SIZE_T
            };

            using DataFileDescription = std::vector<std::pair<DataType, void*>>;


            static void convertFromStream(std::ifstream& stream, DataFileDescription const& output) {

                for(std::pair<DataType, void*> const& element : output) {
                    readFromBuffer(stream, element.first, element.second);
                }

            }
 
            static void convertIntoStream(std::ofstream& stream, DataFileDescription const& output) {

                for(std::pair<DataType, void*> const& element : output) {
                    writeIntoBuffer(stream, element.first, element.second);
                }

            }


        private:

            static void readFromBuffer(std::ifstream& stream, DataType type, void* storage, bool humanReadable = false) {

                switch (type) {

                    case DataType::VEC3:
                    {
                        if (humanReadable) {
                            glm::vec3* vec3Ptr = static_cast<glm::vec3*>(storage);
                            stream >> vec3Ptr->x >> vec3Ptr->y >> vec3Ptr->z;
                            break;
                        }

                        stream.read(reinterpret_cast<char*>(storage), sizeof(float) * 3);

                        break;
                    }
                    case DataType::IVEC3:
                    {

                        if (humanReadable) {
                            glm::ivec3* vec3Ptr = static_cast<glm::ivec3*>(storage);
                            stream >> vec3Ptr->x >> vec3Ptr->y >> vec3Ptr->z;
                            break;
                        }

                        stream.read(reinterpret_cast<char*>(storage), sizeof(int) * 3);

                        break;
                    }
                    case DataType::SVO:
                    {

                        mav::SparseVoxelOctree* svoPtr = static_cast<mav::SparseVoxelOctree*>(storage);

                        svoPtr->readFromFile(stream);

                        // if (humanReadable) {
                            
                        //     stream << vec3Ptr->x << " " << vec3Ptr->y << " " << vec3Ptr->z << std::endl;
                        //     break;
                        // }

                        // stream.write(reinterpret_cast<const char*>(storage), sizeof(int) * 3);

                        break;
                    }
                    case DataType::INT:

                        if (humanReadable) {
                            stream >> *static_cast<int*>(storage);
                            break;
                        }

                        stream.read(reinterpret_cast<char*>(storage), sizeof(int));

                        break;
                    
                    case DataType::SIZE_T:

                        if (humanReadable) {
                            stream >> *static_cast<size_t*>(storage);
                            break;
                        }

                        stream.read(reinterpret_cast<char*>(storage), sizeof(size_t));

                        break;

                    case DataType::FLOAT:
                    {

                        if (humanReadable) {
                            stream >> *static_cast<float*>(storage);
                            break;
                        }

                        stream.read(reinterpret_cast<char*>(storage), sizeof(float));
                        break;
                    }
                    default:
                        throw std::invalid_argument("Invalid requested type.");
                }

            }

            static void writeIntoBuffer(std::ofstream& stream, DataType type, void* storage, bool humanReadable = false) {

                switch (type) {

                    case DataType::VEC3:
                    {

                        if (humanReadable) {
                            glm::vec3* vec3Ptr = static_cast<glm::vec3*>(storage);
                            stream << vec3Ptr->x << " " << vec3Ptr->y << " " << vec3Ptr->z << std::endl;
                            break;
                        }

                        stream.write(reinterpret_cast<const char*>(storage), sizeof(float) * 3);

                        break;
                    }
                    case DataType::IVEC3:
                    {

                        if (humanReadable) {
                            glm::ivec3* vec3Ptr = static_cast<glm::ivec3*>(storage);
                            stream << vec3Ptr->x << " " << vec3Ptr->y << " " << vec3Ptr->z << std::endl;
                            break;
                        }

                        stream.write(reinterpret_cast<const char*>(storage), sizeof(int) * 3);

                        break;
                    }
                    case DataType::SVO:
                    {

                        mav::SparseVoxelOctree* svoPtr = static_cast<mav::SparseVoxelOctree*>(storage);

                        svoPtr->writeToFile(stream);

                        // if (humanReadable) {
                            
                        //     stream << vec3Ptr->x << " " << vec3Ptr->y << " " << vec3Ptr->z << std::endl;
                        //     break;
                        // }

                        // stream.write(reinterpret_cast<const char*>(storage), sizeof(int) * 3);

                        break;
                    }
                    case DataType::INT:

                        if (humanReadable) {
                            stream << *reinterpret_cast<int*>(storage) << std::endl;
                            break;
                        }

                        stream.write(reinterpret_cast<const char*>(storage), sizeof(int));

                        break;
                    
                    case DataType::SIZE_T:

                        if (humanReadable) {
                            stream << *reinterpret_cast<size_t*>(storage) << std::endl;
                            break;
                        }

                        stream.write(reinterpret_cast<const char*>(storage), sizeof(size_t));

                        break;

                    case DataType::FLOAT:

                        if (humanReadable) {
                            stream << *reinterpret_cast<float*>(storage) << std::endl;
                            break;
                        }

                        stream.write(reinterpret_cast<const char*>(storage), sizeof(float));

                        break;

                    default:
                        throw std::invalid_argument("Invalid requested type.");
                }

            }

    };

}
