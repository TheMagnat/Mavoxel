#pragma once

#include <GraphicObjects/Drawable.hpp>


namespace mav {

    class Lines : public Drawable {

        public:
            Lines(const Camera* cameraPtr) : Drawable(), camera(cameraPtr), lastUpdatePosition_(0) {}
            //{{3}, {3}}
            void generateVertices() {}

            void generateVertices(glm::vec3 const& point) {

                vertices_.push_back(point.x);
                vertices_.push_back(point.y);
                vertices_.push_back(point.z);

                vertices_.push_back(1.0f);
                vertices_.push_back(1.0f);
                vertices_.push_back(1.0f);

                if (lastUpdatePosition_) {
                    indices_.push_back(lastUpdatePosition_ - 1);
                    indices_.push_back(lastUpdatePosition_);
                }

                ++lastUpdatePosition_;

            }

            /**
             * 
            */
            void addPoint(glm::vec3 const& point) {
                generateVertices(point);
            }

            void addLine(std::pair<glm::vec3, glm::vec3> const& line, glm::vec3 const& color) {
                
                //Position point 1
                vertices_.push_back(line.first.x);
                vertices_.push_back(line.first.y);
                vertices_.push_back(line.first.z);

                //Color point 1
                vertices_.push_back(color.x);
                vertices_.push_back(color.y);
                vertices_.push_back(color.z);

                //Position point 2
                vertices_.push_back(line.second.x);
                vertices_.push_back(line.second.y);
                vertices_.push_back(line.second.z);

                //Color point 2
                vertices_.push_back(color.x);
                vertices_.push_back(color.y);
                vertices_.push_back(color.z);

                //Indices
                indices_.push_back(lastUpdatePosition_++);
                indices_.push_back(lastUpdatePosition_++);

            }

            void clear() {
                vertices_.clear();
                indices_.clear();
                lastUpdatePosition_ = 0;
            }
            
			std::vector<uint32_t> getVertexAttributesSizes() const override {
                return {3, 3};
            }

            void updateShader(vuw::Shader* shader, uint32_t currentFrame) override {
                //Binding 0
                ViewProjectionObject vp{};
                vp.view = camera->GetViewMatrix();
                vp.projection = camera->Projection;
                vp.projection[1][1] *= -1;

                shader->updateUniform(0, currentFrame, &vp, sizeof(vp));
            }

        private:
            const Camera* camera;

            std::vector<glm::vec3> points_;
            size_t lastUpdatePosition_;
    };

}
