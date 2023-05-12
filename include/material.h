#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

#include "common.h"
#include "hittable.h"
#include "rttexture.h"
#include "onb.h"

class material
{
public:
    virtual bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered, float& pdf) const { return false; }
    virtual float scattering_pdf( const ray& r_in, const hit_record& rec, const ray& scattered ) const {  return 0; }
    virtual glm::vec3 emitted(const ray& r_in, const hit_record& rec, float u, float v, const glm::vec3& p) const { return glm::vec3(0, 0, 0); }
};

class lambertian : public material
{
public:
    lambertian(const glm::vec3& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<rttexture> a) : albedo(a) {}

    virtual bool scatter( const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered, float& pdf) const override
    {
        //glm::vec3 scatter_direction = rec.normal + random_unit_vector();
        //if (near_zero(scatter_direction))
        //    scatter_direction = rec.normal;

        //scattered = ray(rec.p, scatter_direction);
        //attenuation = albedo->value(rec.u, rec.v, rec.p);

        onb uvw;
        uvw.build_from_w(rec.normal);
        auto direction = uvw.local(random_cosine_direction());

        scattered = ray(rec.p, glm::normalize(direction), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        //pdf = dot(rec.normal, scattered.direction()) / pi;
        pdf = dot(uvw.w(), scattered.direction()) / pi;
        
        return true;
    }

    float scattering_pdf( const ray& r_in, const hit_record& rec, const ray& scattered) const {
        auto cosine = glm::dot(rec.normal, glm::normalize(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
    }

public:
    shared_ptr<rttexture> albedo;
};

class metal : public material
{
public:
    metal(const glm::vec3& a, float f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter( const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered, float& pdf ) const override
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

    virtual bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered, float& pdf) const override 
    {
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

class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<rttexture> a) : emit(a) {}
    diffuse_light(glm::vec3 c) : emit(make_shared<solid_color>(c)) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered, float& pdf) const override 
    {
        return false;
    }

    virtual glm::vec3 emitted(const ray& r_in, const hit_record& rec, float u, float v, const glm::vec3& p) const override
    {
        if (rec.front_face)
            return emit->value(u, v, p);
        else
            return glm::vec3(0, 0, 0);
    }

public:
    shared_ptr<rttexture> emit;
};

class isotropic : public material {
public:
    isotropic(glm::vec3 c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<rttexture> a) : albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, glm::vec3& attenuation, ray& scattered, float& pdf) const override 
    {
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<rttexture> albedo;
};

#endif