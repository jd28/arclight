#include "renderservice.h"

#include <nw/kernel/Strings.hpp>
#include <nw/model/Mdl.hpp>

#if defined(_WIN32)
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

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
    , textures_{512}
{

#if defined(_WIN32)
    device_type_ = Diligent::RENDER_DEVICE_TYPE_D3D12;
    auto d3d12Factory = Diligent::GetEngineFactoryD3D12();
    engine_factory_ = d3d12Factory;
#elif defined(__APPLE__)
    device_type_ = Diligent::RENDER_DEVICE_TYPE_METAL;
    auto metalEngineFactory = Diligent::GetEngineFactoryMtl();
    engine_factory_ = metalEngineFactory;
#else
    device_type_ = Diligent::RENDER_DEVICE_TYPE_VULKAN;
    auto vkEngineFactory = Diligent::GetEngineFactoryVk();
    engine_factory_ = vkEngineFactory;
#endif

    if (!engine_factory_) {
        throw std::runtime_error("Failed to get engine factory for the selected graphics API");
    }

#if defined(_WIN32)
    Diligent::EngineD3D12CreateInfo createInfo;
    createInfo.Features.SeparablePrograms = Diligent::DEVICE_FEATURE_STATE_ENABLED;
    createInfo.EnableValidation = true;
    d3d12Factory->CreateDeviceAndContextsD3D12(
        createInfo,
        &device_,
        &immediate_ctx_);
#elif defined(__APPLE__)
    Diligent::EngineMtlCreateInfo createInfo;
    metalEngineFactory->CreateDeviceAndContextsMtl(
        createInfo,
        &device_,
        &immediate_ctx_);
#else
    Diligent::EngineVkCreateInfo createInfo;
    vkEngineFactory->CreateDeviceAndContextsVk(
        createInfo,
        &device_,
        &immediate_ctx_);
#endif

    if (!device_) {
        throw std::runtime_error("Failed to create render device");
    }

    shaders_ = ShaderManager(device_);

    shaders_.load(nw::kernel::strings().intern("basic_vs"),
        Diligent::SHADER_TYPE_VERTEX,
        R"(struct VSInput
        {
            float3 Position : ATTRIB0;
            float2 TexCoord : ATTRIB1;
            float3 Normal   : ATTRIB2;
            float4 Tangent  : ATTRIB3;
        };
        
        struct PSInput
        {
            float4 Position : SV_POSITION;
            float2 TexCoord : TEX_COORD;
            uint TexIndex : TEX_INDEX;
        };
        
        cbuffer Constants : register(b0)
        {
            float4x4 g_Model;
            float4x4 g_View;
            float4x4 g_Projection;
            uint g_TexIndex;
            uint3 g_Padding;
        };
        
        void main(in  VSInput VSIn,
                  out PSInput PSIn)
        {
            PSIn.Position = mul(g_Projection, mul(g_View, mul(g_Model, float4(VSIn.Position, 1.0))));
            PSIn.TexCoord = VSIn.TexCoord;
            PSIn.TexIndex = g_TexIndex;
        })");

    shaders_.load(nw::kernel::strings().intern("skin_vs"),
        Diligent::SHADER_TYPE_VERTEX,
        R"(
        struct VS_INPUT {
            float3 aPos : ATTRIB0;
            float2 aTexCoord : ATTRIB1;
            float3 aNormal : ATTRIB2;
            float4 aTangent : ATTRIB3;
            int4 aIndices : ATTRIB4;
            float4 aWeights : ATTRIB5;
        };
        
        struct VS_OUTPUT {
            float4 Position : SV_POSITION;
            float2 TexCoord : TEX_COORD;
            uint TexIndex : TEX_INDEX;
        };
        
        cbuffer Constants : register(b0) {
            float4x4 model;
            float4x4 view;
            float4x4 projection;
            uint g_TexIndex;
            uint3 g_Padding;
        };
        
        static const int MAX_BONES = 64;
        cbuffer Joints : register(b1) {
            float4x4 joints[MAX_BONES];
        };
        
        VS_OUTPUT main(VS_INPUT input)
        {
            VS_OUTPUT output;
            
            float4 localPosition = float4(0.0, 0.0, 0.0, 0.0);
            
            [unroll]
            for(int i = 0; i < 4; i++)
            {
                if(input.aIndices[i] == -1)
                    continue;
                    
                if(input.aIndices[i] >= MAX_BONES)
                {
                    localPosition = float4(input.aPos, 1.0);
                    break;
                }
                
                float4x4 boneTransform = joints[input.aIndices[i]];
                float4 posePosition = mul(boneTransform, float4(input.aPos, 1.0));
                localPosition += posePosition * input.aWeights[i];
            }
            
            output.Position = mul(projection, mul(view, mul(model, localPosition)));
            output.TexCoord = input.aTexCoord;
            output.TexIndex = g_TexIndex;
            
            return output;
        })");

    shaders_.load(nw::kernel::strings().intern("basic_ps"),
        Diligent::SHADER_TYPE_PIXEL,
        R"(
        Texture2D<float4> g_Textures[512] : register(t0);
        SamplerState g_Texture_sampler;
        
        struct PSInput
        {
            float4 Position : SV_POSITION;
            float2 TexCoord : TEX_COORD;
            uint TexIndex : TEX_INDEX;
        };
        
        float4 main(in PSInput PSIn) : SV_Target
        {
            return g_Textures[PSIn.TexIndex].Sample(g_Texture_sampler, PSIn.TexCoord);
        })");
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
    if (time != nw::kernel::ServiceInitTime::kernel_start
        && time != nw::kernel::ServiceInitTime::module_pre_load) {
        return;
    }

    textures_.load_placeholder();
    textures_.load_palette_texture();
    textures_.load_samplers();
}

std::pair<RenderService::pso_type, RenderService::srb_type> RenderService::get_pso(const RenderPipelineState& rps)
{
    auto hash = rps.hash();
    auto it = pso_map_.find(hash);
    if (it != std::end(pso_map_)) {
        return it->second;
    }

    RenderService::pso_type pso;
    RenderService::srb_type srb;

    Diligent::GraphicsPipelineStateCreateInfo pso_ci;
    auto& pso_desc = pso_ci.PSODesc;
    pso_desc.Name = "Mesh Rendering PSO";
    pso_desc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    pso_ci.pVS = renderer().shaders().get(nw::kernel::strings().intern(rps.has_skin ? "skin_vs" : "basic_vs"));
    pso_ci.pPS = renderer().shaders().get(nw::kernel::strings().intern("basic_ps"));
    if (!pso_ci.pVS || !pso_ci.pPS) {
        LOG_F(ERROR, "Failed to get shaders for PSO");
        return {};
    }

    std::vector<Diligent::LayoutElement> layout_elements;
    if (!rps.has_skin) {
        layout_elements = {
            {0, 0, 3, Diligent::VT_FLOAT32, false, offsetof(nw::model::Vertex, position)},
            {1, 0, 2, Diligent::VT_FLOAT32, false, offsetof(nw::model::Vertex, tex_coords)},
            {2, 0, 3, Diligent::VT_FLOAT32, false, offsetof(nw::model::Vertex, normal)},
            {3, 0, 4, Diligent::VT_FLOAT32, false, offsetof(nw::model::Vertex, tangent)},
        };
    } else {
        layout_elements = {
            {0, 0, 3, Diligent::VT_FLOAT32, false, offsetof(nw::model::SkinVertex, position)},
            {1, 0, 2, Diligent::VT_FLOAT32, false, offsetof(nw::model::SkinVertex, tex_coords)},
            {2, 0, 3, Diligent::VT_FLOAT32, false, offsetof(nw::model::SkinVertex, normal)},
            {3, 0, 4, Diligent::VT_FLOAT32, false, offsetof(nw::model::SkinVertex, tangent)},
            {4, 0, 4, Diligent::VT_INT32, false, offsetof(nw::model::SkinVertex, bones)},
            {5, 0, 4, Diligent::VT_FLOAT32, false, offsetof(nw::model::SkinVertex, weights)},
        };
    }

    pso_ci.GraphicsPipeline.InputLayout.LayoutElements = layout_elements.data();
    pso_ci.GraphicsPipeline.InputLayout.NumElements = static_cast<Diligent::Uint32>(layout_elements.size());

    pso_ci.GraphicsPipeline.NumRenderTargets = 1;
    pso_ci.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
    pso_ci.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D32_FLOAT;
    pso_ci.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pso_ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    pso_ci.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = true;
    pso_ci.GraphicsPipeline.DepthStencilDesc.DepthFunc = Diligent::COMPARISON_FUNC_LESS;

    // Rasterizer state (example)
    pso_ci.GraphicsPipeline.RasterizerDesc.FillMode = Diligent::FILL_MODE_SOLID;
    pso_ci.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
    pso_ci.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;

    std::vector<Diligent::ShaderResourceVariableDesc> vars;
    if (rps.has_skin) {
        vars = {
            {Diligent::SHADER_TYPE_VERTEX, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {Diligent::SHADER_TYPE_VERTEX, "Joints", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {Diligent::SHADER_TYPE_PIXEL, "g_Textures", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_Texture_sampler", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        };
    } else {
        vars = {
            {Diligent::SHADER_TYPE_VERTEX, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {Diligent::SHADER_TYPE_PIXEL, "g_Textures", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_Texture_sampler", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        };
    }
    pso_ci.PSODesc.ResourceLayout.Variables = vars.data();
    pso_ci.PSODesc.ResourceLayout.NumVariables = static_cast<Diligent::Uint32>(vars.size());

    renderer().device()->CreateGraphicsPipelineState(pso_ci, &pso);
    if (!pso) {
        LOG_F(ERROR, "Failed to create PSO");
        return {};
    }

    pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture_sampler")->Set(textures().default_sampler);

    pso->CreateShaderResourceBinding(&srb, true);
    if (!srb) {
        LOG_F(ERROR, "Failed to create SRB");
        return {};
    }

    std::vector<Diligent::IDeviceObject*> textureViewPtrs(textures().texture_views.size());
    for (size_t i = 0; i < textures().texture_views.size(); ++i) {
        textureViewPtrs[i] = textures().texture_views[i].RawPtr();
    }

    srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")
        ->SetArray(textureViewPtrs.data(),
            0,
            static_cast<Diligent::Uint32>(textureViewPtrs.size()),
            Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);

    if (pso && srb) {
        pso_map_.insert({hash, {pso, srb}});
    }

    return {pso, srb};
}

void RenderService::pre_frame()
{
    renderer().immediate_context()->WaitForIdle();

    if (textures().dirty()) {
        std::vector<Diligent::IDeviceObject*> textureViewPtrs(textures().texture_views.size());
        for (size_t i = 0; i < textures().texture_views.size(); ++i) {
            textureViewPtrs[i] = textures().texture_views[i].RawPtr();
        }

        for (auto& [_, v] : pso_map_) {
            v.second->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")
                ->SetArray(textureViewPtrs.data(),
                    0,
                    static_cast<Diligent::Uint32>(textureViewPtrs.size()),
                    Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
        }
        textures().set_dirty(false);
    }
}

std::string RenderService::device_type_as_string() const
{
    switch (device_type_) {
    case Diligent::RENDER_DEVICE_TYPE_D3D12:
        return "DirectX 12";
    case Diligent::RENDER_DEVICE_TYPE_METAL:
        return "Metal";
    case Diligent::RENDER_DEVICE_TYPE_VULKAN:
        return "Vulkan";
    default:
        return "Unknown";
    }
}
