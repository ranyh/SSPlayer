#ifndef __PLAYOS_SHADER_MANAGER_H__
#define __PLAYOS_SHADER_MANAGER_H__

#include <unordered_map>

#include "shader.h"


namespace playos {

class ShaderManager {
private:
    ShaderManager() { }

    ShaderManager(const ShaderManager &) = delete;
    ShaderManager &operator=(ShaderManager &) = delete;

public:
    static ShaderManager &instance();

    void init();

    void registerShader(const std::string &name, std::shared_ptr<Shader> shader);
    std::shared_ptr<Shader> getShader(const std::string &name);
    std::shared_ptr<Shader> defaultShader();

private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
};

}

#endif
