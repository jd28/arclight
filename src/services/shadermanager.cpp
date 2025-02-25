#include "shadermanager.h"

ShaderManager::ShaderManager(Diligent::IRenderDevice* device)
    : device_{device}
{
}

Diligent::IShader* ShaderManager::get(nw::InternedString name) const
{
    auto it = shaders_.find(name);
    return it != shaders_.end() ? it->second : nullptr;
}

Diligent::IShader* ShaderManager::load(nw::InternedString name, Diligent::SHADER_TYPE shaderType, const std::string& source)
{
    Diligent::IShader* result = nullptr;

    return result;
}
