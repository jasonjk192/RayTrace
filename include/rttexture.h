#ifndef RTTEXTURE_H
#define RTTEXTURE_H

#include "common.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "perlin.h"



class rttexture {
public:
    virtual glm::vec3 value(float u, float v, const glm::vec3& p) const = 0;
};

class solid_color : public rttexture
{
public:
    solid_color() {}
    solid_color(glm::vec3 c) : color_value(c) {}

    solid_color(float red, float green, float blue)
        : solid_color(glm::vec3(red, green, blue)) {}

    virtual glm::vec3 value(float u, float v, const glm::vec3& p) const override
    {
        return color_value;
    }

private:
    glm::vec3 color_value;
};

class checker_texture : public rttexture {
public:
    checker_texture() {}

    checker_texture(shared_ptr<rttexture> _even, shared_ptr<rttexture> _odd)
        : even(_even), odd(_odd) {}

    checker_texture(glm::vec3 c1, glm::vec3 c2)
        : even(make_shared<solid_color>(c1)), odd(make_shared<solid_color>(c2)) {}

    virtual glm::vec3 value(float u, float v, const glm::vec3& p) const override
    {
        auto sines = sin(10 * p.x) * sin(10 * p.y) * sin(10 * p.z);
        if (sines < 0)
            return odd->value(u, v, p);
        else
            return even->value(u, v, p);
    }

public:
    shared_ptr<rttexture> odd;
    shared_ptr<rttexture> even;
};

class image_texture : public rttexture {
public:
    const static int bytes_per_pixel = 3;

    image_texture()
        : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

    image_texture(const char* filename)
    {
        auto components_per_pixel = bytes_per_pixel;

        data = stbi_load(filename, &width, &height, &components_per_pixel, components_per_pixel);

        if (!data) {
            std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
            width = height = 0;
        }

        bytes_per_scanline = bytes_per_pixel * width;
    }

    ~image_texture() {
        delete data;
    }

    virtual glm::vec3 value(float u, float v, const glm::vec3& p) const override 
    {
        // If we have no texture data, then return solid cyan as a debugging aid.
        if (data == nullptr)
            return glm::vec3(0, 1, 1);

        // Clamp input texture coordinates to [0,1] x [1,0]
        u = clamp(u, 0.0, 1.0);
        v = 1.0 - clamp(v, 0.0, 1.0);  // Flip V to image coordinates

        auto i = static_cast<int>(u * width);
        auto j = static_cast<int>(v * height);

        // Clamp integer mapping, since actual coordinates should be less than 1.0
        if (i >= width)  i = width - 1;
        if (j >= height) j = height - 1;

        const auto color_scale = 1.0 / 255.0;
        auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;

        return glm::vec3(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
    }

private:
    unsigned char* data;
    int width, height;
    int bytes_per_scanline;
};

class noise_texture : public rttexture {
public:
    noise_texture(): scale(1.f) {}
    noise_texture(float sc) : scale(sc) {}

    virtual glm::vec3 value(float u, float v, const glm::vec3& p) const override 
    {
        return glm::vec3(1, 1, 1) * 0.5f * (1.f + noise.noise(scale * p));
    }

public:
    perlin noise;
    float scale;
};

#endif