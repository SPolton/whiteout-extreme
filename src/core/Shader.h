#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Legacy shader class - kept for backward compatibility with Text system
// For new code, use core/render/ShaderProgram instead
namespace legacy {

class Shader
{
public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(std::string vertexPath, std::string fragmentPath);
    Shader(const char* vertexPath, const char* fragmentPath);

    // use/activate the shader
    void use();

    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, glm::mat4& mat) const;
    unsigned int initVAO(float* vertices, int size);
private:
    void checkCompileErrors(unsigned int shader, std::string type);
};

} // namespace legacy
