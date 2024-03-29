// Credits: https://learnopengl.com/

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;
    std::string fragName;
    std::string vertName;

    // Generates shader from scripts stored in strings (with the option to 'include' a header)
    Shader(std::string* vs_text, std::string* ps_text, std::string name, std::string* include_path = nullptr)
    {
        unsigned int vertex, fragment;
        const GLchar* text_ptr[1];
        vertName = name;
        fragName = name;

        if (include_path != nullptr) shader_include(*vs_text, *include_path);
        replace_all(*vs_text, "hash ", "#");
        if (include_path != nullptr) shader_include(*ps_text, *include_path);
        replace_all(*ps_text, "hash ", "#");

        text_ptr[0] = vs_text->c_str();
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, text_ptr, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        text_ptr[0] = ps_text->c_str();
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, text_ptr, NULL);
        glCompileShader(fragment);
        checkCompileErrors(vertex, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(vertex, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    Shader(const char* computePath)
    {
        std::string computeCode;
        std::ifstream cComputeFile;
        cComputeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            cComputeFile.open(computePath);
            std::stringstream cShaderStream;
            cShaderStream << cComputeFile.rdbuf();
            cComputeFile.close();
            computeCode = cShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }

        computeCode = shader_include(computeCode.c_str(), computePath);

        const char* cShaderCode = computeCode.c_str();

        GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        glDeleteShader(compute);
    }

    // constructor generates the shader on the fly
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            std::string path(fragmentPath);
            fragName = path.substr(path.find_last_of("/\\") + 1);
            path = std::string(vertexPath);
            vertName = path.substr(path.find_last_of("/\\") + 1);
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "Error in file: " << fragName << std::endl;
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }

        vertexCode = shader_include(vertexCode.c_str(), vertexPath);
        fragmentCode = shader_include(fragmentCode.c_str(), fragmentPath);

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w) const
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    // utility function for checking shader compilation/linking errors.
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << " in file: " << ((type.compare("VERTEX") == 0) ? vertName : fragName) << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << " in file: " << vertName << "/" << fragName << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

    // replaces all occurences of a string with another
    void replace_all(std::string& str, const std::string& from, const std::string& to)
    {
        size_t start_pos = 0;

        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    // include path for the #include in shader script
    // Technically there is no #include for shader code so this gets replaced with the actual code from the relevant include file
    // It's more convenient for porting the relevant sections for each pass
    void shader_include(std::string& shader, std::string& include_path)
    {
        size_t start_pos = 0;
        std::string include_dir = "#include ";

        while ((start_pos = shader.find(include_dir, start_pos)) != std::string::npos)
        {
            int pos = start_pos + include_dir.length() + 1;
            int length = shader.find("\"", pos);
            std::string file = shader.substr(pos, length - pos);
            std::string content = "";
            std::string app_path = include_path;

            std::ifstream f;
            f.open((app_path + file).c_str());

            if (f.is_open())
            {
                char buffer[1024];

                while (!f.eof())
                {
                    f.getline(buffer, 1024);
                    content += buffer;
                    content += "\n";
                }
            }
            else
            {
                std::cerr << "Couldn't include shader file: " << app_path + file << "\n";
            }

            shader.replace(start_pos, (length + 1) - start_pos, content);
            start_pos += content.length();
        }
    }

    // 2nd version which takes in shader code and shader's path (not the directory)
    // All relevant includes must be in the directory relative to the current shader in order to replace #include 
    std::string shader_include(const char* shaderCode, const char* shaderPath)
    {
        std::string include_path = std::filesystem::path(shaderPath).parent_path().string();
        
        size_t pos = 0;
        std::string include_dir = "#include ";

        std::string modified_code = std::string(shaderCode);
        while ((pos = modified_code.find(include_dir, pos)) != std::string::npos)
        {
            int include_pos_start = pos;
            pos = modified_code.find("\"", pos) + 1;
            int length = modified_code.find("\"", pos);

            std::string file = modified_code.substr(pos, length - pos);
            std::string content = "";
            std::ifstream f;
            f.open((include_path + "//" + file).c_str());
            if (f.is_open())
            {
                char buffer[1024];

                while (!f.eof())
                {
                    f.getline(buffer, 1024);
                    content += buffer;
                    content += "\n";
                }
            }
            else
            {
                std::cerr << "Couldn't include shader file: " << include_path + "//" + file << "\n";
            }

            modified_code.replace(include_pos_start, (length + 1) - include_pos_start, content);
            pos = include_pos_start + 1;
        }

        return modified_code;
    }
};
#endif