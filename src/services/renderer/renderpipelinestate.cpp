#include "renderpipelinestate.h"

#include <xxhash/xxh3.h>

size_t RenderPipelineState::hash() const
{
    return XXH3_64bits(this, sizeof(*this));
}
