#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

#include "common.h"
#include "hittable.h"

class material
{
public:
    virtual bool scatter( const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered ) const = 0;
};

class lambertian : public material
{
public:
    lambertian(const glm::vec3& a) : albedo(a) {}

    virtual bool scatter( const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered) const override
    {
        glm::vec3 scatter_direction = rec.normal + random_unit_vector();

        if (near_zero(scatter_direction))
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

public:
    glm::vec3 albedo;
};

class metal : public material
{
public:
    metal(const glm::vec3& a, float f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter( const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered ) const override
    {
        glm::vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return (glm::dot(scattered.direction(), rec.normal) > 0);
    }

public:
    glm::vec3 albedo;
    float fuzz;
};

class dielectric : public material {
public:
    dielectric(float index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered
    ) const override {
        attenuation = glm::vec3(1.0, 1.0, 1.0);
        float refraction_ratio = rec.front_face ? (1.f / ir) : ir;

        glm::vec3 unit_direction = glm::normalize(r_in.direction());
        float cos_theta = fmin(glm::dot(-unit_direction, rec.normal), 1.f);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.f;
        glm::vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_float())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        scattered = ray(rec.p, direction);
        return true;
    }

public:
    float ir; // Index of Refraction

private:
    static float reflectance(float cosine, float ref_idx) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1.f - ref_idx) / (1.f + ref_idx);
        r0 = r0 * r0;
        return r0 + (1.f - r0) * pow((1 - cosine), 5);
    }
};

#endif