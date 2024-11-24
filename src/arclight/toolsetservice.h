#ifndef TOOLSETSERVICE_H
#define TOOLSETSERVICE_H

#include "nw/config.hpp"
#include "nw/kernel/Kernel.hpp"

struct PartModels {
    nw::String part;
    char race = 'h';
    int phenotype = 0;
    bool female = false;
    int number = 0;

    auto operator<=>(const PartModels&) const noexcept = default;
    bool operator==(const PartModels&) const noexcept = default;
};

struct ToolsetService : public nw::kernel::Service {
    const static std::type_index type_index;

    ToolsetService(nw::MemoryResource* memory);

    virtual ~ToolsetService() = default;

    virtual void initialize(nw::kernel::ServiceInitTime time) override;

    nw::Vector<PartModels> body_part_models;
};

inline ToolsetService& toolset()
{
    auto res = nw::kernel::services().get_mut<ToolsetService>();
    if (!res) {
        throw std::runtime_error("kernel: unable to load resources service");
    }
    return *res;
}

#endif // TOOLSETSERVICE_H
