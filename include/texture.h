#ifndef TEXTURE_H
#define TEXTURE_H
#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <iostream>

#include <stb_image.h>

class Texture
{
public:
    int width, height, nrChannels;
    std::filesystem::path texPath;
    std::string type;
    unsigned int ID;

    Texture():
        Texture(1.f, 1.f, 1.f)
    { }

    ~Texture() {}

    Texture(int frameWidth, int frameHeight, GLenum type = GL_UNSIGNED_BYTE):
        width(frameWidth),
        height(frameHeight),
        nrChannels(3),
        texPath("NONE (BLANK)")
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight, 0, GL_RGB, type, NULL);
    }

    Texture(int frameWidth, int frameHeight, std::string s) :
        width(frameWidth),
        height(frameHeight),
        nrChannels(4),
        texPath("NONE (BLANK)")
    {
        glGenTextures(1, &ID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, frameWidth, frameHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(0, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    }

    Texture(float r, float g, float b, GLenum type = GL_UNSIGNED_BYTE) :
        width(1),
        height(1),
        nrChannels(3),
        texPath("NONE (COLOR)(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")")
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        unsigned char* data = new unsigned char[3 * sizeof(unsigned char)];
        data[0] = (unsigned char)(r * 255.0f);
        data[1] = (unsigned char)(g * 255.0f);
        data[2] = (unsigned char)(b * 255.0f);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, type, data);
    }

    Texture(std::filesystem::path texturePath, bool flipVertical = false, GLenum type = GL_UNSIGNED_BYTE):
        texPath(texturePath)
    {
        LoadSTBI(flipVertical, type);
    }

    void WriteColorData(unsigned char* data)
    {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }

private:
    void LoadSTBI(bool flipVertical, GLenum type)
    {
        stbi_set_flip_vertically_on_load(flipVertical);

        if (unsigned char* data = stbi_load(texPath.string().c_str(), &width, &height, &nrChannels, 0))
        {
            glGenTextures(1, &ID);
            glBindTexture(GL_TEXTURE_2D, ID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, type, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load texture: " << texPath << std::endl;
            std::cout << "Returned a white 1x1 texture " << std::endl;
            Texture();
        }
    }
};
#endif //TEXTURE_H