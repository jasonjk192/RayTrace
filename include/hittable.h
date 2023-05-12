#ifndef HITTABLE_H
#define HITTABLE_H

#include <glm/glm.hpp>

#include "ray.h"
#include "common.h"
#include "aabb.h"

class material;

struct hit_record {
    glm::vec3 p;
    glm::vec3 normal;
    shared_ptr<material> mat_ptr;
    float t;
    float u;
    float v;
    bool front_face;

    inline void set_face_normal(const ray& r, const glm::vec3& outward_normal)
    {
        front_face = glm::dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
    virtual bool bounding_box(float time0, float time1, aabb& output_box) const = 0;
    virtual float pdf_value(const glm::vec3& o, const glm::vec3& v) const { return 0.0; }
    virtual glm::vec3 random(const glm::vec3& o) const { return glm::vec3(1, 0, 0); }
};


class translate : public hittable {
public:
    translate(shared_ptr<hittable> p, const glm::vec3& displacement)
        : ptr(p), offset(displacement) {}

    virtual bool hit( const ray& r, float t_min, float t_max, hit_record& rec) const override;
    virtual bool bounding_box(float time0, float time1, aabb& output_box) const override;

public:
    shared_ptr<hittable> ptr;
    glm::vec3 offset;
};

bool translate::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
    ray moved_r(r.origin() - offset, r.direction(), r.time());
    if (!ptr->hit(moved_r, t_min, t_max, rec))
        return false;

    rec.p += offset;
    rec.set_face_normal(moved_r, rec.normal);

    return true;
}

bool translate::bounding_box(float time0, float time1, aabb& output_box) const
{
    if (!ptr->bounding_box(time0, time1, output_box))
        return false;

    output_box = aabb(
        output_box.min() + offset,
        output_box.max() + offset);

    return true;
}

class flip_face : public hittable 
{
public:
    flip_face(shared_ptr<hittable> p) : ptr(p) {}

    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override 
    {
        if (!ptr->hit(r, t_min, t_max, rec))
            return false;

        rec.front_face = !rec.front_face;
        return true;
    }

    virtual bool bounding_box(float time0, float time1, aabb& output_box) const override
    {
        return ptr->bounding_box(time0, time1, output_box);
    }

public:
    shared_ptr<hittable> ptr;
};

#endif