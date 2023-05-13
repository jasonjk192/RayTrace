#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "window.h"
#include "settings.h"
#include "texture.h"

#include "common.h"
#include "camera.h"
#include "material.h"
#include "hittable_list.h"
#include "sphere.h"
#include "ray.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "pdf.h"

float hit_sphere(const glm::vec3& center, double radius, const ray& r)
{
    glm::vec3 oc = r.origin() - center;
    auto a = glm::dot(r.direction(), r.direction());
    auto half_b = glm::dot(oc, r.direction());
    auto c = glm::dot(oc, oc) - radius * radius;
    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) {
        return -1.0;
    }
    else {
        return (-half_b - sqrt(discriminant)) /  a;
    }
}

glm::vec3 ray_color(const ray& r, const glm::vec3& background, const hittable& world, shared_ptr<hittable>& lights, int depth)
{
    hit_record rec;

    if (depth <= 0) return glm::vec3(0, 0, 0);
    if (!world.hit(r, 0.001f, infinity, rec)) return background;

    scatter_record srec;
    glm::vec3 emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, srec)) return emitted;
    if (srec.is_specular) return srec.attenuation * ray_color(srec.specular_ray, background, world, lights, depth - 1);

    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr);

    ray scattered = ray(rec.p, p.generate(), r.time());
    auto pdf_val = p.value(scattered.direction());

    return emitted + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered) * ray_color(scattered, background, world, lights, depth - 1) / pdf_val;

    //if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
    //    return emitted;
    //return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
    
    //glm::vec3 unit_direction = glm::normalize(r.direction());
    //auto t = 0.5f * (unit_direction.y + 1.0f);
    //return (1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
}

hittable_list earth()
{
    auto earth_texture = make_shared<image_texture>("./assets/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(glm::vec3(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

hittable_list simple_light()
{
    hittable_list objects;

    //auto pertext = make_shared<noise_texture>(4);
    auto pertext = glm::vec3(0.8, 0.8, 0.0);
    
    objects.add(make_shared<sphere>(glm::vec3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(glm::vec3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(glm::vec3(4, 4, 4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list first_scene()
{
    hittable_list objects;

    auto material_ground = make_shared<lambertian>(glm::vec3(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(glm::vec3(0.1, 0.2, 0.5));
    auto material_left = make_shared<dielectric>(1.5);
    auto material_right = make_shared<metal>(glm::vec3(0.8, 0.6, 0.2), 0.0);
    auto checker = make_shared<checker_texture>(glm::vec3(0.2, 0.3, 0.1), glm::vec3(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(glm::vec3(0.0, -100.5, -1.0), 100.0, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(glm::vec3(0.0, 0.0, -1.0), 0.5, material_center));
    objects.add(make_shared<sphere>(glm::vec3(-1.0, 0.0, -1.0), 0.5, material_left));
    objects.add(make_shared<sphere>(glm::vec3(-1.0, 0.0, -1.0), -0.4, material_left));
    objects.add(make_shared<sphere>(glm::vec3(1.0, 0.0, -1.0), 0.5, material_right));

    //objects.add(make_shared<sphere>(glm::vec3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list cornell_box()
{
    hittable_list objects;

    auto red = make_shared<lambertian>(glm::vec3(.65, .05, .05));
    auto white = make_shared<lambertian>(glm::vec3(.73, .73, .73));
    auto green = make_shared<lambertian>(glm::vec3(.12, .45, .15));
    auto light = make_shared<diffuse_light>(glm::vec3(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    //objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(glm::vec3(0, 0, 0), glm::vec3(165, 330, 165), white);
    //box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, glm::vec3(265, 0, 295));

    shared_ptr<hittable> box2 = make_shared<box>(glm::vec3(0, 0, 0), glm::vec3(165, 165, 165), white);
    //box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, glm::vec3(130, 0, 65));

    objects.add(box1);
    //objects.add(box2);

    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(glm::vec3(190, 90, 190), 90, glass));

    //objects.add(make_shared<constant_medium>(box1, 0.01, glm::vec3(0, 0, 0)));
    //objects.add(make_shared<constant_medium>(box2, 0.01, glm::vec3(1, 1, 1)));

    return objects;
}

int main()
{
    Window window;

    float lastFrame = 0.f;
    float dt = 0.f;

    //glm::vec3 background(0.5f, 0.7f, 1.0f);
    glm::vec3 background(0.f, 0.f, 0.f);


    // Image
    const int image_width = WINDOW_WIDTH;
    const int image_height = WINDOW_HEIGHT;
    const int samples_per_pixel = 10;
    const int max_depth = 10;

    // World
    hittable_list world = cornell_box();
    shared_ptr<hittable> lights = make_shared<sphere>(glm::vec3(190, 90, 190), 90, shared_ptr<material>()); // make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>());

    //world = earth();

    // Camera

    //Camera camera(glm::vec3(-2, 2, 1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), 90, aspect_ratio);
    //Camera camera(glm::vec3(26, 3, 6), glm::vec3(0, 2, 0), glm::vec3(0, 1, 0), 20, aspect_ratio);
    Camera camera(glm::vec3(278, 278, -800), glm::vec3(278, 278, 0), glm::vec3(0, 1, 0), 40, aspect_ratio);

    // SAMPLING //

    Texture col(WINDOW_WIDTH, WINDOW_HEIGHT);
    unsigned char* data = new unsigned char[WINDOW_WIDTH * WINDOW_HEIGHT * 3];
    for (int j = 0; j < WINDOW_HEIGHT; ++j)
    {
        for (int i = 0; i < WINDOW_WIDTH; ++i)
        {
            glm::vec3 pixel_color(0, 0, 0);

            for (int s = 0; s < samples_per_pixel; ++s)
            {
                auto u = (i + random_float()) / (image_width - 1);
                auto v = (j + random_float()) / (image_height - 1);
                ray r = camera.GetRay(u, v);
                pixel_color += ray_color(r, background, world, lights, max_depth);
            }

            if (pixel_color.r != pixel_color.r) pixel_color.r = 0.0;
            if (pixel_color.g != pixel_color.g) pixel_color.g = 0.0;
            if (pixel_color.b != pixel_color.b) pixel_color.b = 0.0;

            pixel_color.r = sqrt(pixel_color.r / samples_per_pixel);
            pixel_color.g = sqrt(pixel_color.g / samples_per_pixel);
            pixel_color.b = sqrt(pixel_color.b / samples_per_pixel);

            pixel_color.r = clamp(pixel_color.r, 0.f, 0.999f);
            pixel_color.g = clamp(pixel_color.g, 0.f, 0.999f);
            pixel_color.b = clamp(pixel_color.b, 0.f, 0.999f);

            data[0 + i * 3 + j * WINDOW_WIDTH * 3] = pixel_color.r * 256;
            data[1 + i * 3 + j * WINDOW_WIDTH * 3] = pixel_color.g * 256;
            data[2 + i * 3 + j * WINDOW_WIDTH * 3] = pixel_color.b * 256;
        }
    }

    col.WriteColorData(data);

    // ----------- //



    
    while (!glfwWindowShouldClose(window.getWindow()))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        dt = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.composeDearImGuiFrame();
        window.showScene(col.ID);
        ImGui::Render();
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window.getWindow());
        glfwPollEvents();
    }

    glfwTerminate();

	return 0;
}