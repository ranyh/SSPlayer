#include "shader_manager.h"

namespace playos {

ShaderManager &ShaderManager::instance()
{
    static ShaderManager manager;

    return manager;
}

void ShaderManager::init(const std::string &resourceDir)
{
    registerShader("default", Shader::fromFile(resourceDir + "/shaders/3d.vs", resourceDir + "/shaders/base.fs"));
    registerShader("font", Shader::fromFile(resourceDir + "/shaders/3d.vs", resourceDir + "/shaders/font.fs"));
    registerShader("primitive", Shader::fromFile(resourceDir + "/shaders/3d.vs", resourceDir + "/shaders/primitive.fs"));
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
