#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "window.h"
#include "settings.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"
#include "quad.h"
#include "buffer.h"

glm::vec2 lastPos(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

//Enables NVIDIA's GPU
extern "C" { __declspec(dllexport) unsigned long NvOptimusEnablement = useDedicatedGPU ? 0x00000001 : 0; }

char keyOnce[GLFW_KEY_LAST + 1];
#define glfwGetKeyOnce(WINDOW, KEY) (glfwGetKey(WINDOW, KEY) ? (keyOnce[KEY] ? false : (keyOnce[KEY] = true)) : (keyOnce[KEY] = false))

void ShaderSetCamera(Shader& shader, Camera& camera)
{
    shader.setVec3("cam.origin", camera.GetOrigin());
    shader.setVec3("cam.horizontal", camera.GetHorizontal());
    shader.setVec3("cam.vertical", camera.GetVertical());
    shader.setVec3("cam.lower_left_corner", camera.GetLowerLeftCorner());
}

void processInput(GLFWwindow* window, Camera& camera)
{
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, 1);
    }
    if (GLFW_PRESS == glfwGetKeyOnce(window, GLFW_KEY_SPACE))
    {
        if (camera.IsControllable())
        {
            camera.SetCameraControl(false);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
            lastPos = glm::vec2(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
            camera.SetCameraControl(true);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

    }

    glm::vec3 move = glm::vec3(0, 0, 0);
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W))
    {
        move += glm::vec3(-1, 0, 0);
    }
    else if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S))
    {
        move += glm::vec3(1, 0, 0);
    }
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Q))
    {
        move += glm::vec3(0, 0, 1);
    }
    else if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_E))
    {
        move += glm::vec3(0, 0, -1);
    }
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A))
    {
        move += glm::vec3(0, -1, 0);
    }
    else if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D))
    {
        move += glm::vec3(0, 1, 0);
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    glm::vec2 mouseOffset = lastPos - glm::vec2(xpos, ypos);
    lastPos = glm::vec2(xpos, ypos);

    camera.TurnCamera(mouseOffset);
    camera.MoveCamera(move);
}

int main()
{
    Window window;
    float lastFrame = 0.f;
    float dt = 0.f;

    Quad2D quad;
    FrameBuffer fbo;
    Texture framebufferImage(WINDOW_WIDTH, WINDOW_HEIGHT);
    fbo.BindTexture(framebufferImage.ID);

    FrameBuffer sfbo;
    Texture screenImage(WINDOW_WIDTH, WINDOW_HEIGHT);
    sfbo.BindTexture(screenImage.ID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Texture noiseTexture(valueNoiseImagePath);

    Shader raytraceShader(expTraceVertPath.string().c_str(), expTraceFragPath.string().c_str());
    Shader screenShader(expScreenVertPath.string().c_str(), expScreenFragPath.string().c_str());
    
    Camera camera(glm::vec3(-2, 2, 1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), 90, aspect_ratio);
    glm::vec2 screenSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    while (glfwWindowShouldClose(window.getWindow()) == 0)
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        dt = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window.getWindow(), camera);

        fbo.BindFrameBuffer();

        glClearColor(0.f, 0.f, 0.f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        raytraceShader.use();
        ShaderSetCamera(raytraceShader, camera);
        raytraceShader.setInt("Time", int(currentFrame));
        raytraceShader.setVec2("screenSize", screenSize);
        quad.Draw();
        
        sfbo.BindFrameBuffer();

        glClearColor(0.f, 0.f, 0.f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        screenShader.use();
        screenShader.setInt("screenTexture", 0);
        quad.BindTexture(GL_TEXTURE0, framebufferImage.ID);
        quad.Draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        window.composeDearImGuiFrame();
        window.showScene(screenImage.ID);

        ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("GLFW deltaTime FPS: %.1f", 1.f / dt);
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window.getWindow());
        glfwPollEvents();
    }

    return 0;
}