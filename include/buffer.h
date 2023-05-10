#ifndef BUFFER_H
#define BUFFER_H

#include <glad/glad.h>

class FrameBuffer
{
public:
    unsigned int ID;

    FrameBuffer()
    {
        glGenFramebuffers(1, &ID);
    }

    ~FrameBuffer()
    {
        glDeleteFramebuffers(1, &ID);
    }

    void BindTexture(unsigned int textureID, unsigned int attachment = GL_COLOR_ATTACHMENT0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureID, 0);
    }

    void BindRenderBufferObject(unsigned int rboID, unsigned int attachment = GL_DEPTH_STENCIL_ATTACHMENT)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboID);
    }

    void BindFrameBuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }

    void UnbindFrameBuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DrawBuffers(GLenum buffers[], int num)
    {
        glDrawBuffers(num, buffers);
    }
};

class RenderBuffer
{
public:
    unsigned int ID;

    RenderBuffer(int width, int height)
    {
        glGenRenderbuffers(1, &ID);
        glBindRenderbuffer(GL_RENDERBUFFER, ID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }

    ~RenderBuffer()
    {
        glDeleteRenderbuffers(1, &ID);
    }
};

class UniformBuffer
{
public:
    unsigned int ID;

    UniformBuffer()
    {
        glGenBuffers(1, &ID);
    }

    void BindData(void* data, int size, unsigned int usage)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
    }

    void BindUniformBuffer()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
    }

    void UnbindUniformBuffer()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

};

#endif //BUFFER_H