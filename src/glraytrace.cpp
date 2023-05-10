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

glm::vec2 lastPos;

//Enables NVIDIA's GPU
extern "C" { __declspec(dllexport) unsigned long NvOptimusEnablement = useDedicatedGPU ? 0x00000001 : 0; }

void DispatchCompute()
{
    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n",
        work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

    int work_grp_size[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

    printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
        work_grp_size[0], work_grp_size[1], work_grp_size[2]);

    GLint work_grp_inv;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
    printf("max local work group invocations %i\n", work_grp_inv);
}

void ShaderSetCamera(Shader& rayShader, Camera& camera)
{
    rayShader.setVec3("cam.origin", camera.GetOrigin());
    rayShader.setVec3("cam.horizontal", camera.GetHorizontal());
    rayShader.setVec3("cam.vertical", camera.GetVertical());
    rayShader.setVec3("cam.lower_left_corner", camera.GetLowerLeftCorner());
}

void processInput(GLFWwindow* window, Camera& camera)
{
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, 1);
    }
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_SPACE))
    {
        if (camera.IsControllable())
        {
            camera.SetCameraControl(false);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
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

    // This is a dumb way to overload Texture() but it works for this simple situation
    Texture rtColor(WINDOW_WIDTH, WINDOW_HEIGHT, "a");
    DispatchCompute();

    Shader quadShader(cVertPath.string().c_str(), cFragPath.string().c_str());
    Shader rayShader(cCompPath.string().c_str());

    Camera camera(glm::vec3(-2, 2, 1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), 90, aspect_ratio);

    lastPos = glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f);

    while (glfwWindowShouldClose(window.getWindow()) == 0)
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        dt = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window.getWindow(), camera);

        ShaderSetCamera(rayShader, camera);
        rayShader.setInt("Time", int(currentFrame));
        rayShader.use();
        glDispatchCompute((GLuint)WINDOW_WIDTH, (GLuint)WINDOW_HEIGHT, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        window.composeDearImGuiFrame();
        window.showScene(rtColor.ID);

        ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("GLFW deltaTime FPS: %.1f", 1.f/dt);
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window.getWindow());
        glfwPollEvents();
    }

    return 0;
}