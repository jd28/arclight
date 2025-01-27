#include "toolsetservice.h"

#include "nw/kernel/Resources.hpp"
#include "nw/kernel/Rules.hpp"

#include <regex>

const std::type_index ToolsetService::type_index{typeid(ToolsetService)};

ToolsetService::ToolsetService(nw::MemoryResource* memory)
    : nw::kernel::Service(memory)
{
    LOG_F(INFO, "[toolset] creating service");
}

void ToolsetService::initialize(nw::kernel::ServiceInitTime time)
{
    if (time != nw::kernel::ServiceInitTime::module_post_load) {
        return;
    }
    LOG_F(INFO, "[toolset] initializing service");

    // Load all part models
    auto model_filter = [this](const nw::Resource& res) {
        static std::regex pattern(R"(p([mf])([a-z])([0-9]+)_([a-z]+)(\d{3})\.mdl)");
        static std::string filename;

        filename = res.filename();
        std::smatch matches;
        if (std::regex_match(filename, matches, pattern)) {
            body_part_models.push_back(
                PartModels{
                    .part = matches[4].str(),
                    .race = char(std::toupper(matches[2].str()[0])),
                    .phenotype = std::stoi(matches[3].str()),
                    .female = matches[1].str()[0] == 'f',
                    .number = std::stoi(matches[5].str()),
                });
        }
    };
    nw::kernel::resman().visit(model_filter, {nw::ResourceType::mdl});
    std::sort(body_part_models.begin(), body_part_models.end());

    base_item_model = new RuleTypeModel<nw::BaseItemInfo>(&nw::kernel::rules().baseitems.entries);
    class_model = new RuleTypeModel<nw::ClassInfo>(&nw::kernel::rules().classes.entries);
}
