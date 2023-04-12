
#include <glm/matrix.hpp>


namespace {
    
    enum FrustumPlanes
    {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        Count,
        Combinations = Count * (Count - 1) / 2
    };

    template<FrustumPlanes i, FrustumPlanes j>
    struct ij2k
    {
        enum { k = i * (9 - i) / 2 + j - 1 };
    };

    template<FrustumPlanes a, FrustumPlanes b, FrustumPlanes c>
    inline glm::vec3 intersection(std::array<glm::vec4, Count> const& planes, std::array<glm::vec3, Combinations> const& crosses)
    {
        float D = glm::dot(glm::vec3(planes[a]), crosses[ij2k<b, c>::k]);
        glm::vec3 res = glm::mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) *
            glm::vec3(planes[a].w, planes[b].w, planes[c].w);
        return res * (-1.0f / D);
    }

}


std::array<glm::vec3, 8> getFrustumPoints(glm::mat4 m) {

    std::array<glm::vec4, Count> planes;
    std::array<glm::vec3, 8> points;

    m = glm::transpose(m);
    planes[Left]   = m[3] + m[0];
    planes[Right]  = m[3] - m[0];
    planes[Bottom] = m[3] + m[1];
    planes[Top]    = m[3] - m[1];
    planes[Near]   = m[3] + m[2];
    planes[Far]    = m[3] - m[2];

    std::array<glm::vec3, Combinations> crosses = {
        glm::cross(glm::vec3(planes[Left]),   glm::vec3(planes[Right])),
        glm::cross(glm::vec3(planes[Left]),   glm::vec3(planes[Bottom])),
        glm::cross(glm::vec3(planes[Left]),   glm::vec3(planes[Top])),
        glm::cross(glm::vec3(planes[Left]),   glm::vec3(planes[Near])),
        glm::cross(glm::vec3(planes[Left]),   glm::vec3(planes[Far])),
        glm::cross(glm::vec3(planes[Right]),  glm::vec3(planes[Bottom])),
        glm::cross(glm::vec3(planes[Right]),  glm::vec3(planes[Top])),
        glm::cross(glm::vec3(planes[Right]),  glm::vec3(planes[Near])),
        glm::cross(glm::vec3(planes[Right]),  glm::vec3(planes[Far])),
        glm::cross(glm::vec3(planes[Bottom]), glm::vec3(planes[Top])),
        glm::cross(glm::vec3(planes[Bottom]), glm::vec3(planes[Near])),
        glm::cross(glm::vec3(planes[Bottom]), glm::vec3(planes[Far])),
        glm::cross(glm::vec3(planes[Top]),    glm::vec3(planes[Near])),
        glm::cross(glm::vec3(planes[Top]),    glm::vec3(planes[Far])),
        glm::cross(glm::vec3(planes[Near]),   glm::vec3(planes[Far]))
    };

    points[0] = intersection<Left,  Bottom, Near>(planes, crosses);
    points[1] = intersection<Left,  Top,    Near>(planes, crosses);
    points[2] = intersection<Right, Bottom, Near>(planes, crosses);
    points[3] = intersection<Right, Top,    Near>(planes, crosses);
    points[4] = intersection<Left,  Bottom, Far>(planes, crosses);
    points[5] = intersection<Left,  Top,    Far>(planes, crosses);
    points[6] = intersection<Right, Bottom, Far>(planes, crosses);
    points[7] = intersection<Right, Top,    Far>(planes, crosses);

    return points;
}

std::array<std::pair<glm::vec3, glm::vec3>, 12> pointsToLines(std::array<glm::vec3, 8> const& points) {
    
    return {{

        //Near
        {points[0], points[1]},
        {points[1], points[3]},
        {points[3], points[2]},
        {points[2], points[0]},

        //Far
        {points[0 + 4], points[1 + 4]},
        {points[1 + 4], points[3 + 4]},
        {points[3 + 4], points[2 + 4]},
        {points[2 + 4], points[0 + 4]},

        //Link
        {points[0], points[0 + 4]},
        {points[1], points[1 + 4]},
        {points[3], points[3 + 4]},
        {points[2], points[2 + 4]}

    }};

}
