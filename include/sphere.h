#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>

#include "hittable.h"

class sphere : public hittable {
public:
    sphere() {}
    sphere(glm::vec3 cen, float r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;
    virtual bool bounding_box(float time0, float time1, aabb& output_box) const override;
    float pdf_value(const glm::vec3& o, const glm::vec3& v) const override;
    glm::vec3 random(const glm::vec3& o) const override;


public:
    glm::vec3 center;
    float radius;
    shared_ptr<material> mat_ptr;

private:
    static void get_sphere_uv(const glm::vec3& p, float& u, float& v)
    {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

        auto theta = acos(-p.y);
        auto phi = atan2(-p.z, p.x) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }
};

inline glm::vec3 random_to_sphere(float radius, float distance_squared)
{
    auto r1 = random_float();
    auto r2 = random_float();
    auto z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

    auto phi = 2 * pi * r1;
    auto x = cos(phi) * sqrt(1 - z * z);
    auto y = sin(phi) * sqrt(1 - z * z);

    return glm::vec3(x, y, z);
}

bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
    glm::vec3 oc = r.origin() - center;
    auto a = glm::dot(r.direction(), r.direction());
    auto half_b = glm::dot(oc, r.direction());
    auto c = glm::dot(oc, oc) - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    glm::vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;
    get_sphere_uv(outward_normal, rec.u, rec.v);

    return true;
}

bool sphere::bounding_box(float time0, float time1, aabb& output_box) const
{
    output_box = aabb(
        center - glm::vec3(radius, radius, radius),
        center + glm::vec3(radius, radius, radius));
    return true;
}

float sphere::pdf_value(const glm::vec3& o, const glm::vec3& v) const {
    hit_record rec;
    if (!this->hit(ray(o, v), 0.001, infinity, rec))
        return 0;

    glm::vec3 direction = center - o;
    auto cos_theta_max = sqrt(1 - radius * radius / glm::dot(direction, direction));
    auto solid_angle = 2 * pi * (1 - cos_theta_max);

    return  1 / solid_angle;
}

glm::vec3 sphere::random(const glm::vec3& o) const {
    glm::vec3 direction = center - o;
    auto distance_squared = glm::dot(direction, direction);
    onb uvw;
    uvw.build_from_w(direction);
    return uvw.local(random_to_sphere(radius, distance_squared));
}

#endif