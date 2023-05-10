#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <limits>
#include <memory>


// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

// Utility Functions

inline float degrees_to_radians(float degrees) {
    return degrees * pi / 180.0;
}

inline float random_float() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0f);
}

inline float random_float(float min, float max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_float();
}

inline float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

inline static glm::vec3 random_vec3() {
    return glm::vec3(random_float(), random_float(), random_float());
}

inline static glm::vec3 random_vec3(float min, float max) {
    return glm::vec3(random_float(min, max), random_float(min, max), random_float(min, max));
}

glm::vec3 random_in_unit_sphere() {
    while (true) {
        auto p = random_vec3(-1, 1);
        if (glm::dot(p,p) >= 1) continue;
        return p;
    }
}

glm::vec3 random_unit_vector() {
    return glm::normalize(random_in_unit_sphere());
}

glm::vec3 random_in_hemisphere(const glm::vec3& normal) {
    glm::vec3 in_unit_sphere = random_in_unit_sphere();
    if (glm::dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}

bool near_zero(glm::vec3 v) {
    // Return true if the vector is close to zero in all dimensions.
    const auto s = 1e-8;
    return (fabs(v.x) < s) && (fabs(v.y) < s) && (fabs(v.z) < s);
}

glm::vec3 reflect(const glm::vec3& v, const glm::vec3& n) {
    return v - 2.f * glm::dot(v, n) * n;
}

glm::vec3 refract(const glm::vec3& uv, const glm::vec3& n, float etai_over_etat) {
    float cos_theta = fmin(glm::dot(-uv, n), 1.f);
    glm::vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    glm::vec3 r_out_parallel = -sqrt(fabs(1.f - glm::dot(r_out_perp, r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}

// Common Headers

#include "ray.h"

#endif