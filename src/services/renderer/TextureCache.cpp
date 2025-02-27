#include "TextureCache.hpp"

#include "renderservice.h"

#include <nw/kernel/Resources.hpp>

#include <DiligentCore/Graphics/GraphicsTools/interface/GraphicsUtilities.h>
#include <DiligentCore/Graphics/GraphicsTools/interface/TextureUploader.hpp>

std::pair<Diligent::RefCntAutoPtr<Diligent::ITexture>, bool> load_texture(std::string_view resref)
{
    Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
    if (resref == "null") { return {texture, false}; }

    auto data = nw::kernel::resman().demand_in_order(resref,
        {nw::ResourceType::dds, nw::ResourceType::plt, nw::ResourceType::tga});

    if (data.bytes.size() == 0) {
        LOG_F(ERROR, "Failed to locate image: {}", resref);
        return {texture, false};
    }

    if (data.name.type != nw::ResourceType::plt) {
        nw::Image img{std::move(data)};
        if (!img.valid()) {
            LOG_F(ERROR, "Failed to load image: {}.{}", resref, nw::ResourceType::to_string(data.name.type));
            return {texture, false};
        }

        Diligent::TextureDesc TexDesc;
        TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        TexDesc.Name = data.name.resref.view().data();
        TexDesc.Width = img.width();
        TexDesc.Height = img.height();
        TexDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
        TexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        TexDesc.Usage = Diligent::USAGE_DEFAULT;
        TexDesc.MipLevels = 1;

        std::vector<uint8_t> rgba_data;
        const uint8_t* src_data = nullptr;
        uint32_t stride = 0;

        if (img.channels() == 3) {

            rgba_data.resize(img.width() * img.height() * 4);
            const uint8_t* rgb_data = img.data();

            for (size_t i = 0; i < img.width() * img.height(); ++i) {
                rgba_data[i * 4 + 0] = rgb_data[i * 3 + 0];
                rgba_data[i * 4 + 1] = rgb_data[i * 3 + 1];
                rgba_data[i * 4 + 2] = rgb_data[i * 3 + 2];
                rgba_data[i * 4 + 3] = 255;
            }

            src_data = rgba_data.data();
            stride = img.width() * 4;
        } else {

            src_data = img.data();
            stride = img.width() * img.channels();
        }

        Diligent::TextureSubResData SubResData;
        SubResData.pData = src_data;
        SubResData.Stride = stride;

        Diligent::TextureData TexData;
        TexData.pSubResources = &SubResData;
        TexData.NumSubresources = 1;

        renderer().device()->CreateTexture(TexDesc, &TexData, &texture);
        return {texture, false};
    } else {
        nw::Plt plt{std::move(data)};
        if (!plt.valid()) {
            LOG_F(ERROR, "Failed to load image: {}.{}", resref, nw::ResourceType::to_string(data.name.type));
            return {texture, false};
        }

        Diligent::TextureDesc TexDesc;
        TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        TexDesc.Width = plt.width();
        TexDesc.Height = plt.height();
        TexDesc.Format = Diligent::TEX_FORMAT_RG8_UNORM;
        TexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        TexDesc.Usage = Diligent::USAGE_DEFAULT;
        TexDesc.MipLevels = 0;

        Diligent::TextureSubResData SubResData;
        SubResData.pData = plt.pixels();
        SubResData.Stride = plt.width() * 2;

        Diligent::TextureData TexData;
        TexData.pSubResources = &SubResData;
        TexData.NumSubresources = 1;

        renderer().device()->CreateTexture(TexDesc, &TexData, &texture);
        return {texture, true};
    }
}

TextureCache::TextureCache()
{
}

std::optional<std::pair<Diligent::RefCntAutoPtr<Diligent::ITexture>, bool>> TextureCache::load(std::string_view resref)
{
    absl::string_view needle{resref.data(), resref.size()};
    auto it = map_.find(needle);
    if (it == std::end(map_)) {
        auto [texture, is_plt] = load_texture(resref);
        if (!texture) {
            return std::make_pair(place_holder_, false);
        }
        auto i = map_.emplace(resref, TexturePayload{{}, texture, is_plt, 1});
        return std::make_pair(i.first->second.handle_, i.first->second.is_plt);
    } else {
        ++it->second.refcount_;
        return std::make_pair(it->second.handle_, it->second.is_plt);
    }
}

void TextureCache::load_palette_texture()
{
    for (size_t i = 0; i < 10; ++i) {
        auto img = nw::kernel::resman().palette_texture(static_cast<nw::PltLayer>(i));
        if (!img->valid()) { continue; }
        Diligent::TextureDesc TexDesc;
        TexDesc.Name = "Palette";
        TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        TexDesc.Width = img->width();
        TexDesc.Height = img->height();
        TexDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
        TexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        TexDesc.Usage = Diligent::USAGE_IMMUTABLE;
        TexDesc.MipLevels = 1;

        std::vector<uint8_t> rgba_data;
        const uint8_t* src_data = nullptr;
        uint32_t stride = 0;

        if (img->channels() == 3) {
            rgba_data.resize(img->width() * img->height() * 4);
            const uint8_t* rgb_data = img->data();

            for (size_t j = 0; j < img->width() * img->height(); ++j) {
                rgba_data[j * 4 + 0] = rgb_data[j * 3 + 0];
                rgba_data[j * 4 + 1] = rgb_data[j * 3 + 1];
                rgba_data[j * 4 + 2] = rgb_data[j * 3 + 2];
                rgba_data[j * 4 + 3] = 255;
            }

            src_data = rgba_data.data();
            stride = img->width() * 4;
        } else {
            src_data = img->data();
            stride = img->width() * img->channels();
        }

        Diligent::TextureSubResData SubResData;
        SubResData.pData = src_data;
        SubResData.Stride = stride;

        Diligent::TextureData TexData;
        TexData.pSubResources = &SubResData;
        TexData.NumSubresources = 1;

        Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
        renderer().device()->CreateTexture(TexDesc, &TexData, &texture);
        palette_texture_[i] = texture;
    }
}

void TextureCache::load_placeholder()
{
    place_holder_image_ = std::make_unique<nw::Image>(":/resrouces/images/templategrid_albedo.png");

    if (place_holder_image_->valid()) {
        Diligent::TextureDesc TexDesc;
        TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        TexDesc.Width = place_holder_image_->width();
        TexDesc.Height = place_holder_image_->height();
        TexDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
        TexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        TexDesc.Usage = Diligent::USAGE_IMMUTABLE;
        TexDesc.MipLevels = 0;

        Diligent::TextureSubResData SubResData;
        SubResData.pData = place_holder_image_->data();
        SubResData.Stride = place_holder_image_->width() * place_holder_image_->channels();

        Diligent::TextureData TexData;
        TexData.pSubResources = &SubResData;
        TexData.NumSubresources = 1;

        renderer().device()->CreateTexture(TexDesc, &TexData, &place_holder_);
    }
}

void TextureCache::load_samplers()
{
    Diligent::SamplerDesc sam_desc;

    sam_desc.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
    sam_desc.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
    sam_desc.MinFilter = Diligent::FILTER_TYPE_LINEAR;
    sam_desc.MagFilter = Diligent::FILTER_TYPE_LINEAR;
    sam_desc.MipFilter = Diligent::FILTER_TYPE_LINEAR;
    renderer().device()->CreateSampler(sam_desc, &default_sampler);

    sam_desc.MinFilter = Diligent::FILTER_TYPE_POINT;
    sam_desc.MagFilter = Diligent::FILTER_TYPE_POINT;
    sam_desc.MipFilter = Diligent::FILTER_TYPE_POINT;
    renderer().device()->CreateSampler(sam_desc, &nearest_sampler);

    sam_desc.AddressU = Diligent::TEXTURE_ADDRESS_CLAMP;
    sam_desc.AddressV = Diligent::TEXTURE_ADDRESS_CLAMP;
    sam_desc.MinFilter = Diligent::FILTER_TYPE_LINEAR;
    sam_desc.MagFilter = Diligent::FILTER_TYPE_LINEAR;
    sam_desc.MipFilter = Diligent::FILTER_TYPE_LINEAR;
    renderer().device()->CreateSampler(sam_desc, &clamp_sampler);

    sam_desc.MinFilter = Diligent::FILTER_TYPE_ANISOTROPIC;
    sam_desc.MagFilter = Diligent::FILTER_TYPE_ANISOTROPIC;
    sam_desc.MipFilter = Diligent::FILTER_TYPE_ANISOTROPIC;
    sam_desc.MaxAnisotropy = 16;
    renderer().device()->CreateSampler(sam_desc, &aniso_sampler);

    sam_desc.AddressU = Diligent::TEXTURE_ADDRESS_CLAMP;
    sam_desc.AddressV = Diligent::TEXTURE_ADDRESS_CLAMP;
    sam_desc.ComparisonFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL;
    sam_desc.MinFilter = Diligent::FILTER_TYPE_COMPARISON_LINEAR;
    sam_desc.MagFilter = Diligent::FILTER_TYPE_COMPARISON_LINEAR;
    sam_desc.MipFilter = Diligent::FILTER_TYPE_COMPARISON_LINEAR;
    renderer().device()->CreateSampler(sam_desc, &shadow_sampler);
}
