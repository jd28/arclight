#ifndef TOOLSETPROFILE_H
#define TOOLSETPROFILE_H

#include "../services/toolset/toolsetservice.h"

#include "nw/profiles/nwn1/Profile.hpp"

struct ToolsetProfile : public nwn1::Profile {
    virtual void load_custom_services() override
    {
        nw::kernel::services().add<ToolsetService>();
    }
};

#endif // TOOLSETPROFILE_H
