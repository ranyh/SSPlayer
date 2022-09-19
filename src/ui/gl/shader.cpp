#include "shader.h"
#include <fstream>
#include <sstream>


Shader::Shader()
{
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

static std::string getContentFromFile(const std::string &path)
{
    std::ifstream fs;
    std::stringstream ss;

    fs.open(path);
    ss << fs.rdbuf();
    fs.close();

    return ss.str();
}

std::shared_ptr<Shader> Shader::fromFile(const std::string &vertexPath, const std::string &fragmentPath)
{
    return fromString(getContentFromFile(vertexPath), getContentFromFile(fragmentPath));
}

std::shared_ptr<Shader> Shader::fromString(const std::string &vertex, const std::string &fragment)
{
    GLint vertexId, fragmentId;
    GLint fShaderSize = fragment.length();
    int success;
    std::shared_ptr<Shader> shader(new Shader());

    vertexId = shader->compileShader(vertex, GL_VERTEX_SHADER);
    fragmentId = shader->compileShader(fragment, GL_FRAGMENT_SHADER);

    shader->ID = glCreateProgram();
    glAttachShader(shader->ID, vertexId);
    glAttachShader(shader->ID, fragmentId);
    glLinkProgram(shader->ID);

    glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetProgramInfoLog(shader->ID, 512, NULL, infoLog);
        printf("Failed to link program: %s\n", infoLog);

        return nullptr;
    }
    glDeleteShader(vertexId);
    glDeleteShader(fragmentId);

    return shader;
}

void Shader::use() 
{ 
    glUseProgram(ID);
}

GLint Shader::compileShader(const std::string &src, GLint type)
{
    GLint id;
    int success;
    const char *_src = src.c_str();

    id = glCreateShader(type);
    glShaderSource(id, 1, &_src, NULL);
    glCompileShader(id);

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        printf("Faild to compile shader: %s\n", infoLog);
    };

    return id;
}
void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &vec) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &vec) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}
