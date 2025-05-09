#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

// ��������� ��� AABB (Axis-Aligned Bounding Box)
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

#endif