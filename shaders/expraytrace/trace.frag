#version 430 core

#define MAT_DIFFUSE 0
#define MAT_METAL 1
#define MAT_DIELECTRIC 2
#define MAT_LIGHT 3

#define PI 3.1415926538
#define INV_SQRT_OF_2PI 0.39894228040143267793994605993439  // 1.0/SQRT_OF_2PI
#define INV_PI 0.31830988618379067153776752674503

const float inf = 99999.f;
const int samples_per_pixel = 100;
const int max_depth = 20;
const uint k = 1103515245U;
float gSeed = 0.0;

#include "common.glsl"
#include "random.glsl"
#include "onb.glsl"
#include "scene_elements.glsl"

out vec4 FragColor;
in vec2 TexCoord;

uniform Camera cam;
uniform vec2 screenSize;
uniform float Time;

/////////// Scene /////////////

bool hit_world(Ray r, float t_min, float t_max, inout HitRecord rec)
{
    Sphere s1 = Sphere(vec3(0.0,0.0,0.0), 0.5);
    Sphere s2 = Sphere(vec3(0.0,-100.5,-1.0), 100);
    Sphere s3 = Sphere(vec3(-1.0, 0.0, -1.0), 0.5);
    Sphere s4 = Sphere(vec3(1.0, 0.0, -1.0), 0.5);
    Sphere s5 = Sphere(vec3(0.0, 1.0, -1.0), 0.1);

    Box b1 = Box(vec3(-0.5, -1.5,-2.5), vec3(1.5, 0,-1.5), rotationAxisAngle( normalize(vec3(1.0,1.0,0.0)), Time ));

    Plane p1 = Plane( 0.5, vec3(0,1,0));

    bool hasHit = false;
    rec.t = t_max;

    if(hit_sphere(s1, r, t_min, rec.t, rec))
    {
        hasHit = true;
        rec.material = Material(MAT_DIFFUSE, vec3(0.9,0.1,0.4), 1.0, 0.0);
    }

    /*if(hit_sphere(s2, r, t_min, rec.t, rec))
    {
        hasHit = true;
        rec.material = Material(MAT_METAL, vec3(0.8,0.8,0), 1.0, 0.0);
    }*/

    if(hit_plane(p1, r, t_min, rec.t, rec))
    {
        hasHit = true;
        rec.material = Material(MAT_DIFFUSE, vec3(0.4,0.9,0.0), 1.0, 0.0);
    }

    if(hit_sphere(s3, r, t_min, rec.t, rec))
    {
        hasHit = true;
        rec.material = Material(MAT_METAL, vec3(0.7,0.4,0.9), 0.5, 0.0);
    }

    if(hit_sphere(s4, r, t_min, rec.t, rec))
    {
        hasHit = true;
        rec.material = Material(MAT_DIELECTRIC, vec3(1,0.8,1), 0.0, 1.5);
    }

    if(hit_box(b1, r, t_min, rec.t, rec))
    {
        hasHit = true;
        rec.material = Material(MAT_METAL, vec3(0.4,0.8,0.9), 0.1, 1.5);
    }

    if(hit_sphere(s5, r, t_min, rec.t, rec))
    {
        hasHit = true;
        rec.material = Material(MAT_LIGHT, vec3(10.0), 0.0, 1.5);
    }

    return hasHit;
}

///////////////////////////////

//////// Ray Tracing //////////

vec3 get_sky(vec3 dir)
{
    //return vec3(0);
    float t = 0.5*(dir.y + 1.0);
    return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
}

float reflectance(float cosine, float ref_idx)
{
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

float ray_material_scattering_pdf(Ray r, HitRecord rec, Ray scattered)
{
    float cosine = dot(rec.normal, normalize(scattered.direction));
            return cosine < 0 ? 0 : cosine/PI;
    return 0;
}

vec3 ray_material_emit(Ray r, HitRecord rec)
{
    if(rec.material.type == 3)
        return rec.material.albedo;
    return vec3(0);
}

bool ray_material_scatter(Ray r, HitRecord rec, inout float seed, out ScatterRecord srec)
{
    if(rec.material.type == MAT_DIFFUSE)
    {
        vec3[3] uvw = build_onb_from_w(rec.normal);
        vec3 local_direction = onb_local(uvw, random_cosine_direction(seed));
        srec.is_specular = false;
        srec.attenuation = rec.material.albedo;
        return true;
    }

    if(rec.material.type == MAT_METAL)
    {
        vec3 reflection = reflect(normalize(r.direction), rec.normal);
        srec.specular_ray = Ray(rec.p, normalize(reflection + rec.material.fuzz * random_in_unit_sphere(gSeed)), r.time);
        srec.attenuation = rec.material.albedo;
        srec.is_specular = true;
        return true;
    }

    if(rec.material.type == MAT_DIELECTRIC)
    {
        float refraction_ratio = rec.front_face ? (1.0/rec.material.ir) : rec.material.ir;
        vec3 unit_direction = normalize(r.direction);
        float cos_theta = min(dot(-unit_direction, rec.normal), 1.0);
        float sin_theta = sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;
        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > hash1(gSeed))
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        srec.specular_ray = Ray(rec.p, normalize(direction), r.time);
        srec.is_specular = true;
        srec.attenuation = rec.material.albedo;
        return true;
    }

    if(rec.material.type == MAT_LIGHT)
    {
        srec.attenuation = rec.material.albedo;
        srec.is_specular = false;
        return false;
    }

    return false;
}

vec3 ray_color(Ray r, inout float seed)
{
    vec3 color = vec3(1.0);
    HitRecord rec;
    for(int i = 0; i < max_depth; ++i)
    {
        if(hit_world(r, 0.001, inf, rec))
        {
            vec3 emit_color = ray_material_emit(r, rec);
            ScatterRecord srec;

            if(ray_material_scatter(r, rec, seed, srec))
            {
                if(srec.is_specular)
                {
                    color *= srec.attenuation;
                    r = srec.specular_ray;
                }
                else
                {

                    /*vec3[3] uvw_for_lambert = build_onb_from_w(rec.normal);
                    vec3 scatter_direction = pdf_cosine_generate(uvw_for_lambert, seed);
                    Ray scattered = Ray(rec.p, scatter_direction, r.time);
                    float pdf = pdf_cosine_value(uvw_for_lambert, scatter_direction);*/

                    Sphere s = Sphere(vec3(0.0, 1.0, -1.0), 0.1);


                    vec3 scatter_direction = pdf_hittable_sphere_generate(s, rec.p, seed) ;
                    Ray scattered = Ray(rec.p, scatter_direction, r.time);
                    float pdf = pdf_hittable_sphere_value(s, rec.p, scatter_direction);

                    color *= srec.attenuation * ray_material_scattering_pdf(r, rec, scattered) / pdf;
                    color += emit_color;
                    r = scattered;
                }
            }
            else return color * emit_color;
        }
        else return color * get_sky(normalize(r.direction));
    }
    return color;
}

///////////////////////////////

void main()
{
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);
    gSeed = float(baseHash(floatBitsToUint(TexCoord))) / float(0xffffffffU) + Time;

	Ray r;
	r.origin = cam.origin;

    for(int s=0; s<samples_per_pixel; s++)
    {
        vec2 n = hash2(gSeed);
        float u = TexCoord.x + n.x / (float(screenSize.x) - 1);
	    float v = TexCoord.y + n.y / (float(screenSize.y) - 1);

        r.direction = cam.lower_left_corner + u * cam.horizontal + v * cam.vertical - cam.origin;

        pixel.rgb += ray_color(r, gSeed);
    }
    pixel.r = isnan(pixel.r)? 0 : pixel.r;
    pixel.g = isnan(pixel.g)? 0 : pixel.g;
    pixel.b = isnan(pixel.b)? 0 : pixel.b;

    pixel.rgb = sqrt(pixel.rgb/samples_per_pixel);
	FragColor = pixel;
}