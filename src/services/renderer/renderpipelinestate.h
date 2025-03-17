#pragma once

#include <compare>

struct RenderPipelineState {
    bool has_diffuse = false;

    // Skinning
    bool has_skin = false;

    auto operator<=>(const RenderPipelineState&) const = default;

    size_t hash() const;
};
