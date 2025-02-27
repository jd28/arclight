#include "shadermanager.h"

#include <nw/util/error_context.hpp>

#include <DiligentCore/Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp>

ShaderManager::ShaderManager(Diligent::IRenderDevice* device)
    : device_{device}
{
}

Diligent::IShader* ShaderManager::get(nw::InternedString name) const
{
    auto it = shaders_.find(name);
    return it != shaders_.end() ? it->second : nullptr;
}

Diligent::IShader* ShaderManager::load(nw::InternedString name, Diligent::SHADER_TYPE type, const std::string& source,
    const std::vector<std::pair<std::string, std::string>>& macros)
{
    auto ns = std::string(name.view());

    Diligent::ShaderCreateInfo shaderCI;
    shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    shaderCI.Desc.ShaderType = type;
    shaderCI.Desc.Name = ns.c_str();
    shaderCI.Source = source.c_str();

    Diligent::IShader* shader = nullptr;
    Diligent::ShaderMacroHelper macro_helper;
    for (const auto& [name, value] : macros) {
        macro_helper.AddShaderMacro(name.c_str(), value.c_str());
    }
    shaderCI.Macros = macro_helper;

    try {
        device_->CreateShader(shaderCI, &shader);

        if (shader) {
            auto [it, inserted] = shaders_.emplace(name, shader);
            if (!inserted) {
                it->second->Release();
                it->second = shader;
            }
            LOG_F(INFO, "Successfully loaded shader '{}'", name.view());
            return shader;
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "\n{}", nw::get_error_context());
        LOG_F(ERROR, "Failed to create shader '{}': {}", name.view(), e.what());
    }

    return nullptr;
}
