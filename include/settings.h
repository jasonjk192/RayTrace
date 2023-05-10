#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>
#include "camera.h"

// settings
unsigned int WINDOW_WIDTH = 800;
unsigned int WINDOW_HEIGHT = 600;
const float aspect_ratio = 4.f/3.f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightCol(1.f, 1.f, 1.f);

// paths
std::filesystem::path shaderPath = std::filesystem::current_path().append("shaders");

// shaders
std::filesystem::path cVertPath = absolute(shaderPath).append("compute.vert");
std::filesystem::path cFragPath = absolute(shaderPath).append("compute.frag");
std::filesystem::path cCompPath = absolute(shaderPath).append("compute.comp");

// other assets
std::filesystem::path valueNoiseImagePath = std::filesystem::current_path().append("assets/ValueNoise.png");

bool useDedicatedGPU = true;

#endif //SETTINGS_H