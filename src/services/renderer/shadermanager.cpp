#include "shadermanager.h"

#include <nw/util/error_context.hpp>

#include <DiligentCore/Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp>

ShaderManager::ShaderManager(Diligent::IRenderDevice* device)
    : device_{device}
{
}

Diligent::IShader* ShaderManager::get(std::string_view name) const
{
    auto it = shaders_.find(name);
    return it != shaders_.end() ? it->second : nullptr;
}

Diligent::IShader* ShaderManager::load(const std::string& name, Diligent::SHADER_TYPE type, const std::string& source,
    const std::vector<std::pair<std::string, std::string>>& macros)
{
    Diligent::ShaderCreateInfo shaderCI;
    shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    shaderCI.Desc.ShaderType = type;
    shaderCI.Desc.Name = name.c_str();
    shaderCI.Source = source.c_str();

    Diligent::RefCntAutoPtr<Diligent::IShader> shader;
    Diligent::ShaderMacroHelper macro_helper;
    for (const auto& [n, value] : macros) {
        macro_helper.AddShaderMacro(n.c_str(), value.c_str());
    }
    shaderCI.Macros = macro_helper;

    device_->CreateShader(shaderCI, &shader);

    if (shader) {
        auto [it, inserted] = shaders_.emplace(name, shader);
        if (!inserted) {
            it->second->Release();
            it->second = shader;
        }
        LOG_F(INFO, "Successfully loaded shader '{}'", name);
        return shader;
    }
    LOG_F(ERROR, "Failed to create shader '{}'", name);

    return nullptr;
}
