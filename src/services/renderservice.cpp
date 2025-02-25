#include "renderservice.h"

#if defined(_WIN32)
#include <DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h>
#elif defined(__APPLE__)
#include <DiligentCore/Graphics/GraphicsEngineMetal/interface/EngineFactoryMtl.h>
#else // Linux and other platforms
#include <DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>
#endif

const std::type_index RenderService::type_index = std::type_index(typeid(RenderService));

RenderService::RenderService(nw::MemoryResource* memory)
    : nw::kernel::Service(memory)
    , device_type_(Diligent::RENDER_DEVICE_TYPE_UNDEFINED)
{
}

RenderService::~RenderService()
{
    contexts_.clear();

    immediate_ctx_.Release();
    device_.Release();
    engine_factory_.Release();
}

void RenderService::initialize(nw::kernel::ServiceInitTime time)
{
    if (time != nw::kernel::ServiceInitTime::kernel_start) { return; }

#if defined(_WIN32)
    device_type_ = Diligent::RENDER_DEVICE_TYPE_D3D12;
    auto d3d12Factory = Diligent::GetEngineFactoryD3D12();
    engine_factory_ = d3d12Factory;
#elif defined(__APPLE__)
    m_deviceType = Diligent::RENDER_DEVICE_TYPE_METAL;
    auto metalEngineFactory = Diligent::GetEngineFactoryMtl();
    m_engineFactory = metalEngineFactory;
#else
    m_deviceType = Diligent::RENDER_DEVICE_TYPE_VULKAN;
    auto vkEngineFactory = Diligent::GetEngineFactoryVk();
    m_engineFactory = vkEngineFactory;
#endif

    if (!engine_factory_) {
        throw std::runtime_error("Failed to get engine factory for the selected graphics API");
    }

    Diligent::EngineCreateInfo engineCI;
    engineCI.Features.SeparablePrograms = Diligent::DEVICE_FEATURE_STATE_ENABLED;

#if defined(_WIN32)
    Diligent::EngineD3D12CreateInfo createInfo;

    d3d12Factory->CreateDeviceAndContextsD3D12(
        createInfo,
        &device_,
        &immediate_ctx_);
#elif defined(__APPLE__)
    Diligent::EngineMtlCreateInfo createInfo;
    metalEngineFactory->CreateDeviceAndContextsMtl(
        createInfo,
        &m_device,
        &m_immediateContext);
#else
    Diligent::EngineVkCreateInfo createInfo;
    vkEngineFactory->CreateDeviceAndContextsVk(
        createInfo,
        &m_device,
        &m_immediateContext);
#endif

    if (!device_) {
        throw std::runtime_error("Failed to create render device");
    }

    shaders_ = ShaderManager(device_);
}
