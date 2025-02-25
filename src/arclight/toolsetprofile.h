#ifndef TOOLSETPROFILE_H
#define TOOLSETPROFILE_H

#include "../services/renderservice.h"
#include "../services/toolsetservice.h"

#include "nw/profiles/nwn1/Profile.hpp"

struct ToolsetProfile : public nwn1::Profile {
    virtual void load_custom_services() override
    {
        nw::kernel::services().add<ToolsetService>();
        nw::kernel::services().add<RenderService>();
    }
};

#endif // TOOLSETPROFILE_H
