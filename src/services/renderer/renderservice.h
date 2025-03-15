#pragma once

#include "TextureCache.hpp"
#include "renderpipelinestate.h"
#include "shadermanager.h"

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <glm/mat4x4.hpp>

#include <nw/kernel/Kernel.hpp>

#include <DiligentCore/Common/interface/BasicMath.hpp>

#include <string>
#include <unordered_map>

struct RenderContext {
    glm::mat4 view;
    glm::mat4 projection;
};

class RenderService : public nw::kernel::Service {
public:
    const static std::type_index type_index;

    using pso_type = Diligent::RefCntAutoPtr<Diligent::IPipelineState>;
    using srb_type = Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>;

    RenderService(nw::MemoryResource* memory);
    RenderService(RenderService&) = delete;
    RenderService(RenderService&&) = delete;
    RenderService& operator=(RenderService&) = delete;
    RenderService& operator=(RenderService&&) = delete;
    virtual ~RenderService();

    virtual void initialize(nw::kernel::ServiceInitTime time) override;

    /// Get the render device - shared across all contexts
    Diligent::IRenderDevice* device() const noexcept { return device_; }

    /// Get the immediate context - for main thread operations
    Diligent::IDeviceContext* immediate_context() const noexcept { return immediate_ctx_; }

    /// Get the graphics API being used
    Diligent::RENDER_DEVICE_TYPE device_type() const noexcept { return device_type_; }

    /// Get a string representation of the API being used
    std::string device_type_as_string() const;

    /// Gets PSO and SRB
    std::pair<pso_type, srb_type> get_pso(const RenderPipelineState& rps);

    /// Get shader manager
    ShaderManager& shaders() { return shaders_; }
    const ShaderManager& shaders() const { return shaders_; }

    /// Get Texture Cache
    TextureCache& textures() { return textures_; }

    /// Get Texture Cache
    const TextureCache& textures() const noexcept { return textures_; }

private:
    Diligent::RENDER_DEVICE_TYPE device_type_;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> device_;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> immediate_ctx_;
    Diligent::RefCntAutoPtr<Diligent::IEngineFactory> engine_factory_;
    std::unordered_map<void*, RenderContext> contexts_;
    ShaderManager shaders_;
    TextureCache textures_;
    absl::flat_hash_map<uint64_t, std::pair<pso_type, srb_type>> pso_map_;
};

inline RenderService& renderer()
{
    auto res = nw::kernel::services().get_mut<RenderService>();
    if (!res) {
        throw std::runtime_error("kernel: unable to load render service");
    }
    return *res;
}
