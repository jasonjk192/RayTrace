#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "common.h"

class Camera
{
public:
    Camera(glm::vec3 lookfrom, glm::vec3 lookat, glm::vec3 vup, float vfov, float aspect_ratio):
        fov(vfov), aspectRatio(aspect_ratio), worldUp(vup), moveSpeed(0.01f), isCameraControllable(false)
    {
        float theta = degrees_to_radians(fov);
        float h = tan(theta / 2);
        float viewport_height = 2.0 * h;
        float viewport_width = aspectRatio * viewport_height;

        forward = glm::normalize(lookfrom - lookat);
        glm::vec3 w = forward;
        glm::vec3 u = glm::normalize(glm::cross(worldUp, w));
        glm::vec3 v = glm::cross(w, u);

        origin = lookfrom;
        horizontal = viewport_width * u;
        vertical = viewport_height * v;
        lower_left_corner = origin - horizontal / 2.f - vertical / 2.f - w;

        right = u;
        up = v;
    }

    ray GetRay(float u, float v) const {
        return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
    }

    void MoveCamera(glm::vec3 move)
    {
        if (!isCameraControllable) return;
        move *= moveSpeed;
        origin += move.x * forward + move.y * right + move.z * up;
        UpdateCamera();
    }

    void TurnCamera(glm::vec2 offset)
    {
        if (!isCameraControllable) return;
        yaw += offset.x;
        pitch += offset.y;
        forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward.y = sin(glm::radians(pitch));
        forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward = glm::normalize(forward);

        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
        UpdateCamera();
    }

    void SetCameraControl(bool isControllable) { isCameraControllable = isControllable; }

    glm::vec3 GetOrigin() const { return origin; }
    glm::vec3 GetHorizontal() const { return horizontal; }
    glm::vec3 GetVertical() const { return vertical; }
    glm::vec3 GetLowerLeftCorner() const { return lower_left_corner; }
    glm::vec3 GetForward() const { return forward; }
    glm::vec3 GetRight() const { return right; }
    glm::vec3 GetUp() const { return up; }
    bool IsControllable() const { return isCameraControllable; }

private:
    glm::vec3 origin;
    glm::vec3 worldUp;
    glm::vec3 lower_left_corner;
    glm::vec3 horizontal;
    glm::vec3 vertical;

    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;

    float yaw;
    float pitch;

    float fov;
    float aspectRatio;
    float moveSpeed;

    bool isCameraControllable;
    
    void UpdateCamera()
    {
        float theta = degrees_to_radians(fov);
        float h = tan(theta / 2);
        float viewport_height = 2.0 * h;
        float viewport_width = aspectRatio * viewport_height;

        glm::vec3 w = forward;
        glm::vec3 u = right;
        glm::vec3 v = up;

        horizontal = viewport_width * u;
        vertical = viewport_height * v;
        lower_left_corner = origin - horizontal / 2.f - vertical / 2.f - w;
    }
};
#endif