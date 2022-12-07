#include "shader_manager.h"

namespace playos {

ShaderManager &ShaderManager::instance()
{
    static ShaderManager manager;

    return manager;
}

void ShaderManager::init(const std::string &resourceDir)
{
#ifdef __APPLE__
    registerShader("default", Shader::fromFile(
            resourceDir + "/shaders/GL3.3/3d.vs",
            resourceDir + "/shaders/GL3.3/base.fs"));
    registerShader("font", Shader::fromFile(
            resourceDir + "/shaders/GL3.3/3d.vs",
            resourceDir + "/shaders/GL3.3/font.fs"));
    registerShader("primitive", Shader::fromFile(
            resourceDir + "/shaders/GL3.3/3d.vs",
            resourceDir + "/shaders/GL3.3/primitive.fs"));
    registerShader("yuv", Shader::fromFile(
            resourceDir + "/shaders/GL3.3/3d.vs",
            resourceDir + "/shaders/GL3.3/yuv_planar.fs"));
    registerShader("nv12", Shader::fromFile(
            resourceDir + "/shaders/GL3.3/3d.vs",
            resourceDir + "/shaders/GL3.3/nv12.fs"));
#else
    registerShader("default", Shader::fromFile(
            resourceDir + "/shaders/ES2.0/3d.vs",
            resourceDir + "/shaders/ES2.0/base.fs"));
    registerShader("font", Shader::fromFile(
            resourceDir + "/shaders/ES2.0/3d.vs",
            resourceDir + "/shaders/ES2.0/font.fs"));
    registerShader("primitive", Shader::fromFile(
            resourceDir + "/shaders/ES2.0/3d.vs",
            resourceDir + "/shaders/ES2.0/primitive.fs"));
    registerShader("yuv", Shader::fromFile(
            resourceDir + "/shaders/ES2.0/3d.vs",
            resourceDir + "/shaders/ES2.0/yuv_planar.fs"));
    registerShader("nv12", Shader::fromFile(
            resourceDir + "/shaders/ES2.0/3d.vs",
            resourceDir + "/shaders/ES2.0/nv12.fs"));
    registerShader("yuv_ext", Shader::fromFile(
            resourceDir + "/shaders/ES2.0/3d.vs",
            resourceDir + "/shaders/ES2.0/yuv_ext.fs"));
#endif
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
