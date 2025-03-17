#pragma once

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Shader.h>

#include <absl/container/flat_hash_map.h>

#include <string>

class ShaderManager {
public:
    explicit ShaderManager(Diligent::IRenderDevice* device = nullptr);

    /// Get a previously loaded shader
    Diligent::IShader* get(std::string_view name) const;

    /// Load shader from source code strings, handling platform differences
    Diligent::IShader* load(const std::string& name, Diligent::SHADER_TYPE type, const std::string& source,
        const std::vector<std::pair<std::string, std::string>>& macros = {});

private:
    Diligent::IRenderDevice* device_ = nullptr;
    absl::flat_hash_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> shaders_;
};
