
#include <Collision/Frustum.hpp>

#include <GLObject/Camera.hpp>

namespace {

    inline bool isOnOrForwardPlane(const mav::FrustumPlane& plane, const mav::AABB& aabb)
    {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        // here r represent half of the length of the AABB box on the plane space
        const float r = aabb.extents.x * std::abs(plane.normal.x) +
                        aabb.extents.y * std::abs(plane.normal.y) +
                        aabb.extents.z * std::abs(plane.normal.z);

        //If this distance is lower than the distance of the center of the AABB box on the place space, it mean the AABB box is forward
        return -r <= plane.getSignedDistanceToPlane(aabb.center);
    }

}

namespace mav {

    Frustum::Frustum() {}
                
    Frustum::Frustum(const mav::Camera* cam, float fovY, float aspect, float zNear, float zFar)
        : fovY_(fovY), aspect_(aspect), zNear_(zNear), zFar_(zFar)
        , halfVSide(zFar * tanf(fovY * 0.5f)), halfHSide(halfVSide * aspect)
    {
        updateCamera(cam);
    }

    void Frustum::updateAll(const mav::Camera* cam, float fovY, float aspect, float zNear, float zFar) {
        updatePerspective(fovY, aspect, zNear, zFar);
        updateCamera(cam);
    }

    /**
     * Note: Camera will need to be updated too to take into account a perspective change.
    */
    void Frustum::updatePerspective(float fovY, float aspect, float zNear, float zFar) {
        fovY_ = fovY;
        aspect_ = aspect;
        zNear_ = zNear;
        zFar_ = zFar;
        halfVSide = zFar * tanf(fovY * 0.5f);
        halfHSide = halfVSide * aspect;
    }

    void Frustum::updateCamera(const mav::Camera* cam) {
        frontMultFar = zFar_ * cam->Front;

        nearFace = { cam->Position + zNear_ * cam->Front, cam->Front };
        farFace = { cam->Position + frontMultFar, -cam->Front };

        rightFace = { cam->Position, glm::cross(frontMultFar - cam->Right * halfHSide, cam->Up) };

        leftFace = { cam->Position, glm::cross(cam->Up,frontMultFar + cam->Right * halfHSide) };

        topFace = { cam->Position, glm::cross(cam->Right, frontMultFar - cam->Up * halfVSide) };

        bottomFace = { cam->Position, glm::cross(frontMultFar + cam->Up * halfVSide, cam->Right) };

    }

    /**
     * Return true if the aabb box collide with the frustum, false otherwise.
     */
    bool Frustum::collide(const mav::AABB& aabb) const {

        return (
            isOnOrForwardPlane(leftFace, aabb) &&
            isOnOrForwardPlane(rightFace, aabb) &&
            isOnOrForwardPlane(topFace, aabb) &&
            isOnOrForwardPlane(bottomFace, aabb) &&
            isOnOrForwardPlane(nearFace, aabb) &&
            isOnOrForwardPlane(farFace, aabb)
        );

    }

}