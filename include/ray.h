#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

class ray
{
public:
    ray() {}
    ray(const glm::vec3& origin, const glm::vec3& direction, float time = 0.f)
        : orig(origin), dir(direction), tm(time)
    {}

    glm::vec3 origin() const { return orig; }
    glm::vec3 direction() const { return dir; }
    float time() const { return tm; }

    glm::vec3 at(float t) const {
        return orig + t * dir;
    }

public:
    glm::vec3 orig;
    glm::vec3 dir;
    float tm;
};

#endif