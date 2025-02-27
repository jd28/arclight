#pragma once

#include <nw/formats/Image.hpp>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string_view>

class TextureCache {
public:
    TextureCache();
    ~TextureCache() = default;

    std::optional<std::pair<Diligent::RefCntAutoPtr<Diligent::ITexture>, bool>> load(std::string_view resref);

    Diligent::RefCntAutoPtr<Diligent::ISampler> default_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler> nearest_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler> clamp_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler> aniso_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler> shadow_sampler;

    void load_palette_texture();
    void load_placeholder();
    void load_samplers();

private:
    struct TexturePayload {
        absl::string_view key_;
        Diligent::RefCntAutoPtr<Diligent::ITexture> handle_;
        bool is_plt;
        size_t refcount_;
    };

    std::unique_ptr<nw::Image> place_holder_image_;
    Diligent::RefCntAutoPtr<Diligent::ITexture> place_holder_;
    Diligent::RefCntAutoPtr<Diligent::ITexture> palette_texture_[10];
    absl::flat_hash_map<absl::string_view, TexturePayload> map_;
};
