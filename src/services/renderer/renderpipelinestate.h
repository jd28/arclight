#pragma once

#include <xxhash/xxh3.h>

#include <compare>

struct RenderPipelineState {
    bool has_diffuse = false;

    // Skinning
    bool has_skin = false;

    auto operator<=>(const RenderPipelineState&) const = default;

    // Hash function for use in containers
    size_t hash() const
    {
        return XXH3_64bits(this, sizeof(*this));
    }
};
