#pragma once

#include "shadermanager.h"

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

#include "nw/kernel/Kernel.hpp"

#include <string>
#include <unordered_map>

struct RenderContext {
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> swapchain;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> immediate_ctx;
};

class RenderService : public nw::kernel::Service {
public:
    const static std::type_index type_index;

    RenderService(nw::MemoryResource* memory);
    virtual ~RenderService();

    virtual void initialize(nw::kernel::ServiceInitTime time) override;

    /// Create a rendering context for a specific widget/window
    RenderContext* create(void* nativeWindowHandle);

    /// Release a rendering context
    void release(RenderContext* context);

    /// Get the render device - shared across all contexts
    Diligent::IRenderDevice* device() const noexcept { return device_; }

    /// Get the immediate context - for main thread operations
    Diligent::IDeviceContext* immediate_context() const noexcept { return immediate_ctx_; }

    /// Get the graphics API being used
    Diligent::RENDER_DEVICE_TYPE device_type() const noexcept { return device_type_; }

    /// Get a string representation of the API being used
    std::string device_type_as_string() const;

    /// Get shader manager
    ShaderManager& shaders() { return shaders_; }

private:
    Diligent::RENDER_DEVICE_TYPE device_type_;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> device_;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> immediate_ctx_;
    Diligent::RefCntAutoPtr<Diligent::IEngineFactory> engine_factory_;
    std::unordered_map<void*, RenderContext*> contexts_;
    ShaderManager shaders_;
};

// Global accessor function
inline RenderService& renderer()
{
    auto res = nw::kernel::services().get_mut<RenderService>();
    if (!res) {
        throw std::runtime_error("kernel: unable to load render service");
    }
    return *res;
}
