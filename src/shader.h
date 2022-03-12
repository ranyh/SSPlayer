#ifndef __SHADER_MANAGER_H__
#define __SHADER_MANAGER_H__

#include <string>
#include <memory>

#include "glad/glad.h"
#include "glm/matrix.hpp"

class Shader
{
private:
    unsigned int ID;
    // constructor reads and builds the shader
    Shader();

    GLint compileShader(const std::string &src, GLint type);
public:
    ~Shader();
    static std::shared_ptr<Shader> fromFile(const std::string &vertexPath, const std::string &fragmentPath);
    static std::shared_ptr<Shader> fromString(const std::string &vertex, const std::string &fragment);

    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    unsigned int id() { return ID; }
};

#endif
