#include "shader_manager.h"

namespace playos {

ShaderManager &ShaderManager::instance()
{
    static ShaderManager manager;

    return manager;
}

void ShaderManager::init()
{
    registerShader("default", Shader::fromFile("../shaders/3d.vs", "../shaders/base.fs"));
    registerShader("font", Shader::fromFile("../shaders/3d.vs", "../shaders/font.fs"));
    registerShader("color", Shader::fromFile("../shaders/3d.vs", "../shaders/base_color.fs"));
}

void ShaderManager::registerShader(const std::string &name, std::shared_ptr<Shader> shader)
{
    m_shaders[name] = shader;
}

std::shared_ptr<Shader> ShaderManager::getShader(const std::string &name)
{
    return m_shaders[name];
}

std::shared_ptr<Shader> ShaderManager::defaultShader()
{
    return m_shaders["default"];
}

}
