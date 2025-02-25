#pragma once

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Shader.h>

#include "nw/util/InternedString.hpp"

#include <absl/container/flat_hash_map.h>

#include <string>

class ShaderManager {
public:
    explicit ShaderManager(Diligent::IRenderDevice* device = nullptr);

    /// Get a previously loaded shader
    Diligent::IShader* get(nw::InternedString name) const;

    /// Load shader from source code strings, handling platform differences
    Diligent::IShader* load(nw::InternedString name, Diligent::SHADER_TYPE shaderType, const std::string& source);

private:
    Diligent::IRenderDevice* device_ = nullptr;
    absl::flat_hash_map<nw::InternedString, Diligent::RefCntAutoPtr<Diligent::IShader>> shaders_;
};
