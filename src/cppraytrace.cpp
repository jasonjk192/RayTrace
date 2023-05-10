
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

glm::vec3 ray_color(const ray& r, const hittable& world, int depth)
{
    hit_record rec;

    if (depth <= 0)
        return glm::vec3(0, 0, 0);

    if (world.hit(r, 0.001f, infinity, rec))
    {
        ray scattered;
        glm::vec3 attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return glm::vec3(0, 0, 0);
    }
    glm::vec3 unit_direction = glm::normalize(r.direction());
    auto t = 0.5f * (unit_direction.y + 1.0f);
    return (1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
}

int main()
{
    Window window;

    float lastFrame = 0.f;
    float dt = 0.f;

    // Image
    const auto aspect_ratio = 4.0 / 3.0;
    const int image_width = WINDOW_WIDTH;
    const int image_height = WINDOW_HEIGHT;
    const int samples_per_pixel = 4;
    const int max_depth = 40;

    // World
    hittable_list world;
    
    auto material_ground = make_shared<lambertian>(glm::vec3(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(glm::vec3(0.1, 0.2, 0.5));
    auto material_left = make_shared<dielectric>(1.5);
    auto material_right = make_shared<metal>(glm::vec3(0.8, 0.6, 0.2), 0.0);

    world.add(make_shared<sphere>(glm::vec3(0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(glm::vec3(0.0, 0.0, -1.0), 0.5, material_center));
    world.add(make_shared<sphere>(glm::vec3(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(make_shared<sphere>(glm::vec3(-1.0, 0.0, -1.0), -0.4, material_left));
    world.add(make_shared<sphere>(glm::vec3(1.0, 0.0, -1.0), 0.5, material_right));

    // Camera

    Camera cam(90.f, aspect_ratio);

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
                ray r = cam.GetRay(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }

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