#pragma once

#include <glm/vec3.hpp>
#include <glm/gtx/scalar_multiplication.hpp>

#include <Collision/AABB.hpp>

namespace mav {

    class Camera;

    struct FrustumPlane
    {
        glm::vec3 normal;
        float distance;

        FrustumPlane() {};
        FrustumPlane(const glm::vec3& p1, const glm::vec3& norm) : normal(glm::normalize(norm)), distance(glm::dot(normal, p1)) {}
        
        inline float getSignedDistanceToPlane(const glm::vec3& point) const
        {
            return glm::dot(normal, point) - distance;
        }

    };

    class Frustum {

        public:
            Frustum();
            
            Frustum(const mav::Camera* cam, float fovY, float aspect, float zNear, float zFar);
            
            void updateAll(const mav::Camera* cam, float fovY, float aspect, float zNear, float zFar);

            /**
             * Note: Camera will need to be updated too to take into account a perspective change.
            */
            void updatePerspective(float fovY, float aspect, float zNear, float zFar);

            void updateCamera(const mav::Camera* cam);

            /**
             * Return true if the aabb box collide with the frustum, false otherwise.
             */
            bool collide(const mav::AABB& aabb) const;

        
        private:
            FrustumPlane topFace;
            FrustumPlane bottomFace;

            FrustumPlane rightFace;
            FrustumPlane leftFace;

            FrustumPlane farFace;
            FrustumPlane nearFace;

            float halfVSide;
            float halfHSide;
            glm::vec3 frontMultFar;

            float fovY_;
            float aspect_;
            float zNear_;
            float zFar_;

    };

}
