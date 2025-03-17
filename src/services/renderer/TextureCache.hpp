#pragma once

#include <nw/formats/Image.hpp>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string_view>

struct TextureID {
    uint32_t id = 0;
};

class TextureCache {
public:
    TextureCache(uint32_t max_textures);
    ~TextureCache() = default;

    std::pair<TextureID, bool> load(std::string_view resref);

    Diligent::RefCntAutoPtr<Diligent::ISampler> default_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler> nearest_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler> clamp_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler> aniso_sampler;
    Diligent::RefCntAutoPtr<Diligent::ISampler> shadow_sampler;

    bool dirty() const noexcept;
    void set_dirty(bool dirty);

    void load_palette_texture();
    void load_placeholder();
    void load_samplers();

    TextureID allocate_texture_id();
    void release(TextureID tex);

    std::vector<Diligent::RefCntAutoPtr<Diligent::ITextureView>> texture_views;

private:
    struct TexturePayload {
        TextureID id;
        Diligent::RefCntAutoPtr<Diligent::ITexture> handle_;
        bool is_plt;
        size_t refcount_;
    };

    uint32_t max_texture_id_ = 1024;
    uint32_t next_texture_id_ = 0;
    std::vector<TextureID> texture_id_free_list_;

    std::unique_ptr<nw::Image> place_holder_image_;
    Diligent::RefCntAutoPtr<Diligent::ITexture> place_holder_;
    Diligent::RefCntAutoPtr<Diligent::ITexture> palette_texture_[10];
    absl::flat_hash_map<nw::Resref, TexturePayload> map_;

    std::vector<nw::Resref> id_to_resref_;
    bool is_dirty_ = false;
};
