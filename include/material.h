#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

#include "common.h"
#include "hittable.h"
#include "rttexture.h"
#include "onb.h"
#include "pdf.h"

struct scatter_record {
    ray specular_ray;
    bool is_specular;
    glm::vec3 attenuation;
    shared_ptr<pdf> pdf_ptr;
};

class material
{
public:
    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const { return false; }
    virtual float scattering_pdf( const ray& r_in, const hit_record& rec, const ray& scattered ) const {  return 0; }
    virtual glm::vec3 emitted(const ray& r_in, const hit_record& rec, float u, float v, const glm::vec3& p) const { return glm::vec3(0, 0, 0); }
};

class lambertian : public material
{
public:
    lambertian(const glm::vec3& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<rttexture> a) : albedo(a) {}

    virtual bool scatter( const ray& r_in, const hit_record& rec, scatter_record& srec) const override
    {
        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        return true;
    }

    float scattering_pdf( const ray& r_in, const hit_record& rec, const ray& scattered) const 
    {
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

    virtual bool scatter( const ray& r_in, const hit_record& rec, scatter_record& srec ) const override
    {
        glm::vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
        srec.specular_ray = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
        srec.attenuation = albedo;
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        return true;
    }

public:
    glm::vec3 albedo;
    float fuzz;
};

class dielectric : public material {
public:
    dielectric(float index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
    {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = glm::vec3(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        glm::vec3 unit_direction = glm::normalize(r_in.direction());
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        glm::vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_float())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        srec.specular_ray = ray(rec.p, direction, r_in.time());
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

    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
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

    /*virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
    {
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }*/

public:
    shared_ptr<rttexture> albedo;
};

#endif