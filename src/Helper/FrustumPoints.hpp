#pragma once

#include <glm/matrix.hpp>
#include <glm/vec3.hpp>


std::array<glm::vec3, 8> getFrustumPoints(glm::mat4 m);

std::array<std::pair<glm::vec3, glm::vec3>, 12> pointsToLines(std::array<glm::vec3, 8> const& points);
