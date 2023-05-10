#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <iostream>
#include <filesystem>
#include <ctime>

#include "GLFW/glfw3.h"

#include "settings.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class Window
{
public:
    Window()
    {
        CreateWindow();
    }

    ~Window()
    {
        closeImGui();
        glfwTerminate();
    }

    bool CreateWindow()
    {
        window = InitGL();
        if (!window)
            return false;
        initializeImGui();
        return true;
    }

    void getPixel(int x, int y, GLubyte* color)
    {
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
    }

    void composeDearImGuiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void showCameraHUD()
    {
        ImGui::Begin("Camera HUD", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar);
       // ImGui::Text("Enable movement: %s", sceneCamera->enableCameraMovement ? "True" : "False");
        //ImGui::Text("Position: %.3f, %.3f, %.3f", sceneCamera->Position.x, sceneCamera->Position.y, sceneCamera->Position.z);
        //ImGui::Text("Speed: %.3f", sceneCamera->MovementSpeed);
        //ImGui::Text("Sensitivity: %.3f", sceneCamera->MouseSensitivity);
        //ImGui::Text("FOV: %.3f", sceneCamera->fov);
        //ImGui::Text("Pitch: %.3f", sceneCamera->Pitch);
        //ImGui::Text("Yaw: %.3f", sceneCamera->Yaw);
        ImGui::End();
    }

    void showScene(int imageID)
    {
        ImVec2 pos = ImVec2(0, 0);
        ImGui::SetNextWindowPos(pos);
        ImGui::Begin("Scene", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        ImVec2 wsize = ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT);
        ImGui::Image((void*)(intptr_t)imageID, wsize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
    }

    GLFWwindow* getWindow()
    {
        return window;
    }


private:
    GLFWwindow* window;

    GLFWwindow* InitGL()
    {
        // glfw: initialize and configure
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // glfw window creation
        GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Raytrace", NULL, NULL);
        if (window == nullptr)
        {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return nullptr;
        }
        glfwMakeContextCurrent(window);

        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD\n";
            return nullptr;
        }

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        return window;
    }

    bool initializeImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) { return false; }
        if (!ImGui_ImplOpenGL3_Init()) { return false; }
        return true;
    }

    std::string timeToString(std::chrono::system_clock::time_point timePoint)
    {
        time_t t = std::chrono::system_clock::to_time_t(timePoint);
        char str[26];
        ctime_s(str, sizeof str, &t);
        return str;
    }

    void closeImGui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

};

#endif