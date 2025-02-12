#ifndef TOOLSETSERVICE_H
#define TOOLSETSERVICE_H

#include "../widgets/proxymodels.h"
#include "../widgets/statictwodamodel.h"
#include "rulesetmodels.h"

#include "nw/config.hpp"
#include "nw/kernel/Kernel.hpp"
#include "nw/objects/Placeable.hpp"
#include "nw/objects/Trap.hpp"
#include "nw/rules/Class.hpp"
#include "nw/rules/items.hpp"

#include <absl/container/flat_hash_map.h>

class QStringListModel;
class QStandardItemModel;

struct CompositeModels {
    QStandardItemModel* top_model;
    QStandardItemModel* top_color;
    QStandardItemModel* middle_model;
    QStandardItemModel* middle_color;
    QStandardItemModel* bottom_model;
    QStandardItemModel* bottom_color;
};

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
    virtual ~ToolsetService();

    virtual void initialize(nw::kernel::ServiceInitTime time) override;

    CompositeModels get_composite_models(std::string_view type, bool mdl = true);
    QStandardItemModel* get_layered_models(std::string_view type);
    QStandardItemModel* get_simple_models(std::string_view type);

    nw::Vector<PartModels> body_part_models;

    // All the below models should be considered lagically const,
    // once created they need to be moved on the main thread,
    // or issues will arise.
    std::unique_ptr<RuleFilterProxyModel> appearances_filter;
    std::unique_ptr<RuleTypeModel<nw::AppearanceInfo>> appearances_model;
    std::unique_ptr<RuleFilterProxyModel> baseitem_filter;
    std::unique_ptr<RuleTypeModel<nw::BaseItemInfo>> baseitem_model;
    std::unique_ptr<RuleFilterProxyModel> class_filter;
    std::unique_ptr<RuleTypeModel<nw::ClassInfo>> class_model;
    std::unique_ptr<EmptyFilterProxyModel> doortypes_filter;
    std::unique_ptr<StaticTwoDAModel> doortypes_model;
    std::unique_ptr<EmptyFilterProxyModel> genericdoors_filter;
    std::unique_ptr<StaticTwoDAModel> genericdoors_model;
    std::unique_ptr<QStandardItemModel> loadscreens_model;
    std::unique_ptr<RuleFilterProxyModel> phenotype_filter;
    std::unique_ptr<RuleTypeModel<nw::PhenotypeInfo>> phenotype_model;
    std::unique_ptr<RuleFilterProxyModel> placeable_filter;
    std::unique_ptr<RuleTypeModel<nw::PlaceableInfo>> placeable_model;
    std::unique_ptr<RuleFilterProxyModel> race_filter;
    std::unique_ptr<RuleTypeModel<nw::RaceInfo>> race_model;
    std::unique_ptr<QStandardItemModel> faction_model;
    std::unique_ptr<RuleTypeModel<nw::TrapInfo>> trap_model;
    std::unique_ptr<QStandardItemModel> creaturespeed_model;
    std::unique_ptr<QStandardItemModel> packages_model;
    std::unique_ptr<QStandardItemModel> ranges_model;
    std::unique_ptr<QStandardItemModel> tails_model;
    std::unique_ptr<QStandardItemModel> wings_model;

    absl::flat_hash_map<std::string, CompositeModels> composite_model_map;
    absl::flat_hash_map<std::string, std::unique_ptr<QStandardItemModel>> simple_model_map;

    std::unique_ptr<QStringListModel> gender_basic_model;
    std::unique_ptr<QStandardItemModel> dynamic_appearance_model;
    std::unique_ptr<QStandardItemModel> cloak_model;
    std::unique_ptr<QStandardItemModel> parts_belt;
    std::unique_ptr<QStandardItemModel> parts_bicep;
    std::unique_ptr<QStandardItemModel> parts_chest;
    std::unique_ptr<QStandardItemModel> parts_foot;
    std::unique_ptr<QStandardItemModel> parts_forearm;
    std::unique_ptr<QStandardItemModel> parts_hand;
    std::unique_ptr<QStandardItemModel> parts_legs;
    std::unique_ptr<QStandardItemModel> parts_neck;
    std::unique_ptr<QStandardItemModel> parts_pelvis;
    std::unique_ptr<QStandardItemModel> parts_robe;
    std::unique_ptr<QStandardItemModel> parts_shin;
    std::unique_ptr<QStandardItemModel> parts_shoulder;
};

inline ToolsetService& toolset()
{
    auto res = nw::kernel::services().get_mut<ToolsetService>();
    if (!res) {
        throw std::runtime_error("kernel: unable to load toolset service");
    }
    return *res;
}

#endif // TOOLSETSERVICE_H
