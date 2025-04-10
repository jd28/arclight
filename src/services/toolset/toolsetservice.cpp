#include "toolsetservice.h"

#include "nw/kernel/FactionSystem.hpp"
#include "nw/kernel/Resources.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/kernel/TwoDACache.hpp"

#include <absl/container/btree_map.h>
#include <absl/container/btree_set.h>

#include <regex>

#include <QCoreApplication>
#include <QObject>
#include <QPainter>
#include <QStandardItemModel>
#include <QStringListModel>

QImage merge_loadscreen_image(const QImage& image)
{
    int width = image.width();
    int height = image.height();

    int halfHeight = height / 2;

    QImage result(int(width * 1.5), halfHeight, QImage::Format_ARGB32);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.drawImage(QRect(width / 2, 0, width, halfHeight),
        image,
        QRect(0, 0, width, halfHeight));

    painter.drawImage(QRect(0, 0, width, halfHeight),
        image,
        QRect(0, halfHeight, width, halfHeight));
    painter.end();

    return result;
}

const std::type_index ToolsetService::type_index{typeid(ToolsetService)};

ToolsetService::ToolsetService(nw::MemoryResource* memory)
    : nw::kernel::Service(memory)
{
    LOG_F(INFO, "[toolset] creating service");
}

ToolsetService::~ToolsetService()
{
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

    appearances_model.reset(new RuleTypeModel<nw::AppearanceInfo>(&nw::kernel::rules().appearances.entries));
    appearances_filter.reset(new RuleFilterProxyModel());
    appearances_filter->setSourceModel(appearances_model.get());
    appearances_filter->sort(0);
    appearances_model->moveToThread(QCoreApplication::instance()->thread());
    appearances_filter->moveToThread(QCoreApplication::instance()->thread());

    baseitem_model.reset(new RuleTypeModel<nw::BaseItemInfo>(&nw::kernel::rules().baseitems.entries));
    baseitem_filter.reset(new RuleFilterProxyModel());
    baseitem_filter->setSourceModel(baseitem_model.get());
    baseitem_filter->sort(0);
    baseitem_model->moveToThread(QCoreApplication::instance()->thread());
    baseitem_filter->moveToThread(QCoreApplication::instance()->thread());

    class_model.reset(new RuleTypeModel<nw::ClassInfo>(&nw::kernel::rules().classes.entries));
    class_filter.reset(new RuleFilterProxyModel());
    class_filter->setSourceModel(class_model.get());
    class_filter->sort(0);
    class_model->moveToThread(QCoreApplication::instance()->thread());
    class_filter->moveToThread(QCoreApplication::instance()->thread());

    auto doortypes = nw::kernel::twodas().get("doortypes");
    auto name_col = int(doortypes->column_index("StringRefGame"));
    doortypes_model.reset(new StaticTwoDAModel(doortypes, name_col));
    doortypes_model->setColumns({name_col});
    doortypes_filter = std::make_unique<EmptyFilterProxyModel>(0);
    doortypes_filter->setSourceModel(doortypes_model.get());
    doortypes_model->moveToThread(QCoreApplication::instance()->thread());
    doortypes_filter->moveToThread(QCoreApplication::instance()->thread());

    auto genericdoors = nw::kernel::twodas().get("genericdoors");
    name_col = int(genericdoors->column_index("Name"));
    genericdoors_model = std::make_unique<StaticTwoDAModel>(genericdoors, name_col);
    genericdoors_model->setColumns({name_col});
    genericdoors_filter = std::make_unique<EmptyFilterProxyModel>(0);
    genericdoors_filter->setSourceModel(genericdoors_model.get());
    genericdoors_filter->sort(0);
    genericdoors_model->moveToThread(QCoreApplication::instance()->thread());
    genericdoors_filter->moveToThread(QCoreApplication::instance()->thread());

    std::string temp_string;
    int temp_int;
    auto loadscreens = nw::kernel::twodas().get("loadscreens");
    loadscreens_model = std::make_unique<QStandardItemModel>();
    for (size_t i = 0; i < loadscreens->rows(); ++i) {
        QString name;
        if (loadscreens->get_to(i, "StrRef", temp_int)) {
            name = to_qstring(nw::kernel::strings().get(temp_int));
        } else if (loadscreens->get_to(i, "Label", temp_string)) {
            name = to_qstring(temp_string);
        }
        if (name.isEmpty()) { continue; }

        if (!loadscreens->get_to(i, "BMPResRef", temp_string)) { continue; }

        auto resref = nw::Resref{temp_string};
        auto item = new QStandardItem(name);
        auto rdata = nw::kernel::resman().demand_in_order(nw::Resref(resref),
            {nw::ResourceType::dds, nw::ResourceType::tga});

        if (rdata.bytes.size()) {
            nw::Image icon{std::move(rdata)};
            QImage image(icon.release(), icon.width(), icon.height(), icon.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
                [](void* bytes) { if (bytes) { free(bytes); } });

            // stb automatically flips standard dds and tga,
            // so here for bioware dds which is implemented a bit differently
            // needs to be flipped back to right side up.
            if (icon.is_bio_dds()) {
                image.mirror();
            }

            image = merge_loadscreen_image(image);
            item->setIcon(QIcon(QPixmap::fromImage(image)));
        }

        item->setData(int(i));
        loadscreens_model->appendRow(item);
    }
    loadscreens_model->moveToThread(QCoreApplication::instance()->thread());

    phenotype_model.reset(new RuleTypeModel<nw::PhenotypeInfo>(&nw::kernel::rules().phenotypes.entries));
    phenotype_filter.reset(new RuleFilterProxyModel());
    phenotype_filter->setSourceModel(phenotype_model.get());
    phenotype_filter->sort(-1);
    phenotype_model->moveToThread(QCoreApplication::instance()->thread());
    phenotype_filter->moveToThread(QCoreApplication::instance()->thread());

    placeable_model.reset(new RuleTypeModel<nw::PlaceableInfo>(&nw::kernel::rules().placeables.entries));
    placeable_filter.reset(new RuleFilterProxyModel());
    placeable_filter->setSourceModel(placeable_model.get());
    placeable_filter->sort(0);
    placeable_model->moveToThread(QCoreApplication::instance()->thread());
    placeable_filter->moveToThread(QCoreApplication::instance()->thread());

    race_model.reset(new RuleTypeModel<nw::RaceInfo>(&nw::kernel::rules().races.entries));
    race_filter.reset(new RuleFilterProxyModel());
    race_filter->setSourceModel(race_model.get());
    race_filter->sort(0);
    race_model->moveToThread(QCoreApplication::instance()->thread());
    race_filter->moveToThread(QCoreApplication::instance()->thread());

    faction_model = std::make_unique<QStandardItemModel>();
    for (auto& faction : nw::kernel::factions().all()) {
        auto item = new QStandardItem(to_qstring(faction));
        item->setData(nw::kernel::factions().faction_id(faction));
        faction_model->appendRow(item);
    }

    trap_model = std::make_unique<RuleTypeModel<nw::TrapInfo>>(&nw::kernel::rules().traps.entries);
    trap_model->moveToThread(QCoreApplication::instance()->thread());
    trap_filter = std::make_unique<RuleFilterProxyModel>();
    trap_filter->setSourceModel(trap_model.get());
    trap_filter->moveToThread(QCoreApplication::instance()->thread());

    auto creaturespeed = nw::kernel::twodas().get("creaturespeed");
    creaturespeed_model = std::make_unique<QStandardItemModel>();
    for (size_t i = 0; i < creaturespeed->rows(); ++i) {
        int temp;
        if (creaturespeed->get_to(i, "Name", temp)) {
            auto item = new QStandardItem(to_qstring(nw::kernel::strings().get(temp)));
            item->setData(int(i));
            creaturespeed_model->appendRow(item);
        }
    }
    creaturespeed_model->sort(0);
    creaturespeed_model->moveToThread(QCoreApplication::instance()->thread());

    auto packages = nw::kernel::twodas().get("packages");
    packages_model = std::make_unique<QStandardItemModel>();
    for (size_t i = 0; i < packages->rows(); ++i) {
        int name, package_class;
        if (packages->get_to(i, "Name", name)
            && packages->get_to(i, "ClassID", package_class)) {
            auto item = new QStandardItem(to_qstring(nw::kernel::strings().get(name)));
            item->setData(int(i), Qt::UserRole + 1);
            item->setData(package_class, Qt::UserRole + 2);
            packages_model->appendRow(item);
        }
    }
    packages_model->sort(0);
    packages_model->moveToThread(QCoreApplication::instance()->thread());

    auto ranges = nw::kernel::twodas().get("ranges");
    ranges_model = std::make_unique<QStandardItemModel>();
    for (size_t i = 0; i < ranges->rows(); ++i) {
        int temp;
        if (ranges->get_to(i, "Name", temp)) {
            auto item = new QStandardItem(to_qstring(nw::kernel::strings().get(temp)));
            item->setData(int(i));
            ranges_model->appendRow(item);
        }
    }
    ranges_model->sort(0);
    ranges_model->moveToThread(QCoreApplication::instance()->thread());

    sound_model = std::make_unique<SoundModel>();
    sound_model->moveToThread(QCoreApplication::instance()->thread());

    auto tailmodel_2da = nw::kernel::twodas().get("tailmodel");
    tails_model = std::make_unique<QStandardItemModel>();

    {
        auto item = new QStandardItem("-- None --");
        item->setData(0);
        tails_model->appendRow(item);
    }

    for (size_t i = 1; i < tailmodel_2da->rows(); ++i) {
        if (!tailmodel_2da->get_to(i, "LABEL", temp_string) || temp_string.empty()) { continue; }
        auto item = new QStandardItem(to_qstring(temp_string));
        item->setData(int(i));
        tails_model->appendRow(item);
    }
    tails_model->sort(0, Qt::AscendingOrder);
    tails_model->moveToThread(QCoreApplication::instance()->thread());

    auto wingmodel_2da = nw::kernel::twodas().get("wingmodel");
    wings_model = std::make_unique<QStandardItemModel>();

    {
        auto item = new QStandardItem("-- None --");
        item->setData(0);
        wings_model->appendRow(item);
    }

    for (size_t i = 1; i < wingmodel_2da->rows(); ++i) {
        if (!wingmodel_2da->get_to(i, "LABEL", temp_string) || temp_string.empty()) { continue; }
        auto item = new QStandardItem(to_qstring(temp_string));
        item->setData(int(i));
        wings_model->appendRow(item);
    }
    wings_model->sort(0, Qt::AscendingOrder);
    wings_model->moveToThread(QCoreApplication::instance()->thread());

    auto waypoint_2da = nw::kernel::twodas().get("waypoint");
    waypoint_model = std::make_unique<QStandardItemModel>();

    for (size_t i = 1; i < waypoint_2da->rows(); ++i) {
        if (!waypoint_2da->get_to(i, "strref", temp_int)) { continue; }
        auto item = new QStandardItem(to_qstring(nw::kernel::strings().get(uint32_t(temp_int))));
        item->setData(int(i));
        waypoint_model->appendRow(item);
    }
    waypoint_model->moveToThread(QCoreApplication::instance()->thread());

    gender_basic_model.reset(new QStringListModel());
    gender_basic_model->setStringList(QStringList()
        << QStringListModel::tr("Male")
        << QStringListModel::tr("Female"));
    gender_basic_model->moveToThread(QCoreApplication::instance()->thread());

    dynamic_appearance_model.reset(new QStandardItemModel());
    for (size_t i = 0; i < nw::kernel::rules().appearances.entries.size(); ++i) {
        const auto& app = nw::kernel::rules().appearances.entries[i];
        if (!app.valid()) { continue; }
        if (nw::string::icmp(app.model_type, "P")) {
            auto item = new QStandardItem(to_qstring(app.editor_name()));
            item->setData(int(i));
            dynamic_appearance_model->appendRow(item);
        }
    }
    dynamic_appearance_model->sort(-1);
    dynamic_appearance_model->moveToThread(QCoreApplication::instance()->thread());

    cloak_model.reset(new QStandardItemModel);
    auto cloakmodel = nw::kernel::twodas().get("cloakmodel");
    if (cloakmodel) {
        std::string name;
        for (size_t i = 0; i < cloakmodel->rows(); ++i) {
            if (cloakmodel->get_to(i, "LABEL", name)) {
                auto item = new QStandardItem(to_qstring(name));
                item->setData(int(i));
                cloak_model->appendRow(item);
            }
        }
        cloak_model->sort(0);
        cloak_model->moveToThread(QCoreApplication::instance()->thread());
    }

    auto create_part_model = [](const nw::StaticTwoDA* tda) {
        auto result = new QStandardItemModel;
        if (!tda) { return result; }
        float temp;
        size_t col = tda->column_index("ACBONUS");
        if (col == nw::StaticTwoDA::npos) { return result; }

        for (size_t i = 0; i < tda->rows(); ++i) {
            if (tda->get_to(i, col, temp)) {
                auto item = new QStandardItem(QString::number(i));
                item->setData(int(i));
                result->appendRow(item);
            }
        }
        result->moveToThread(QCoreApplication::instance()->thread());
        return result;
    };

    parts_belt.reset(create_part_model(nw::kernel::twodas().get("parts_belt")));
    parts_bicep.reset(create_part_model(nw::kernel::twodas().get("parts_bicep")));
    parts_chest.reset(create_part_model(nw::kernel::twodas().get("parts_chest")));
    parts_foot.reset(create_part_model(nw::kernel::twodas().get("parts_foot")));
    parts_forearm.reset(create_part_model(nw::kernel::twodas().get("parts_forearm")));
    parts_hand.reset(create_part_model(nw::kernel::twodas().get("parts_hand")));
    parts_legs.reset(create_part_model(nw::kernel::twodas().get("parts_legs")));
    parts_neck.reset(create_part_model(nw::kernel::twodas().get("parts_neck")));
    parts_pelvis.reset(create_part_model(nw::kernel::twodas().get("parts_pelvis")));
    parts_robe.reset(create_part_model(nw::kernel::twodas().get("parts_robe")));
    parts_shin.reset(create_part_model(nw::kernel::twodas().get("parts_shin")));
    parts_shoulder.reset(create_part_model(nw::kernel::twodas().get("parts_shoulder")));
}

CompositeModels ToolsetService::get_composite_models(std::string_view type, bool mdl)
{
    std::string search = fmt::format(R"({}_([tmb])_(\d{{3}})\.{})", type,
        mdl ? "mdl" : "(dds|tga)");

    std::regex pattern(search);
    auto it = composite_model_map.find(type);
    if (it != std::end(composite_model_map)) {
        return it->second;
    }

    absl::btree_map<int, absl::btree_set<int>> top_colors_per_model, mid_colors_per_model, bot_colors_per_model;
    absl::btree_map<int, absl::btree_set<int>> color_to_models_top, color_to_models_mid, color_to_models_bot;

    absl::btree_set<int> top_models, mid_models, bot_models;
    int count = 0;

    auto filter = [&](const nw::Resource& res) {
        std::string filename = res.filename();
        std::smatch matches;
        if (std::regex_match(filename, matches, pattern)) {
            int value = std::stoi(matches[2].str());
            char part = matches[1].str()[0];
            int model_id = value / 10;
            int color_id = value % 10;

            if (model_id > 0 && color_id > 0) {
                if (part == 't') {
                    top_models.insert(model_id);
                    top_colors_per_model[model_id].insert(color_id);
                    color_to_models_top[color_id].insert(model_id);
                } else if (part == 'm') {
                    mid_models.insert(model_id);
                    mid_colors_per_model[model_id].insert(color_id);
                    color_to_models_mid[color_id].insert(model_id);
                } else if (part == 'b') {
                    bot_models.insert(model_id);
                    bot_colors_per_model[model_id].insert(color_id);
                    color_to_models_bot[color_id].insert(model_id);
                }
            }
            ++count;
        }
    };

    if (mdl) {
        nw::kernel::resman().visit(filter, {nw::ResourceType::mdl});
    } else {
        nw::kernel::resman().visit(filter, {nw::ResourceType::dds, nw::ResourceType::tga});
    }

    auto create_model = [](const absl::btree_set<int>& set) -> QStandardItemModel* {
        auto model = new QStandardItemModel;
        for (int val : set) {
            auto item = new QStandardItem(QString::number(val));
            item->setData(val);
            model->appendRow(item);
        }
        model->moveToThread(QCoreApplication::instance()->thread());
        return model;
    };

    auto create_color_model = [](const absl::btree_map<int, absl::btree_set<int>>& color_map) -> QStandardItemModel* {
        auto model = new QStandardItemModel;
        for (const auto& [color, models] : color_map) {
            auto item = new QStandardItem(QString::number(color));
            QVariantList mids;
            for (int m : models) {
                mids.append(m);
            }
            item->setData(color);
            item->setData(mids, Qt::UserRole + 2);
            model->appendRow(item);
        }
        model->moveToThread(QCoreApplication::instance()->thread());
        return model;
    };

    CompositeModels models;
    models.top_model = create_model(top_models);
    models.top_color = create_color_model(color_to_models_top);
    models.middle_model = create_model(mid_models);
    models.middle_color = create_color_model(color_to_models_mid);
    models.bottom_model = create_model(bot_models);
    models.bottom_color = create_color_model(color_to_models_bot);

    composite_model_map.insert({std::string(type), models});

    return models;
}

QStandardItemModel* ToolsetService::get_layered_models(std::string_view type)
{
    std::string search = fmt::format(R"({}_(\d{{3}})\.mdl)", type);
    absl::btree_set<int> models;

    std::regex pattern(search);
    auto it = simple_model_map.find(type);
    if (it != std::end(simple_model_map)) {
        return it->second.get();
    }

    auto filter = [&](const nw::Resource& res) {
        std::string filename = res.filename();
        std::smatch matches;
        if (std::regex_match(filename, matches, pattern)) {
            int value = std::stoi(matches[1].str());
            models.insert(value);
        }
    };
    nw::kernel::resman().visit(filter, {nw::ResourceType::mdl});

    auto create_model = [&](absl::btree_set<int>& set) -> QStandardItemModel* {
        auto model = new QStandardItemModel;
        for (auto val : set) {
            auto resref = fmt::format("{}_{:03d}", type, val);
            auto item = new QStandardItem(QString::fromStdString(resref));
            item->setData(val);
            model->appendRow(item);
        }
        return model;
    };

    auto result = create_model(models);
    simple_model_map.emplace(std::string(type), result);
    return result;
}

QStandardItemModel* ToolsetService::get_simple_models(std::string_view type)
{
    std::string search = fmt::format(R"({}_(\d{{3}})\.(dds|tga))", type);
    absl::btree_set<int> models;

    std::regex pattern(search);
    auto it = simple_model_map.find(type);
    if (it != std::end(simple_model_map)) {
        return it->second.get();
    }

    auto filter = [&](const nw::Resource& res) {
        std::string filename = res.filename();
        std::smatch matches;
        if (std::regex_match(filename, matches, pattern)) {
            int value = std::stoi(matches[1].str());
            models.insert(value);
        }
    };

    nw::kernel::resman().visit(filter, {nw::ResourceType::dds, nw::ResourceType::tga});

    auto create_model = [&](absl::btree_set<int>& set) -> QStandardItemModel* {
        auto model = new QStandardItemModel;
        for (auto val : set) {
            auto resref = fmt::format("{}_{:03d}", type, val);
            auto item = new QStandardItem(QString::fromStdString(resref));
            auto rdata = nw::kernel::resman().demand_in_order(nw::Resref(resref),
                {nw::ResourceType::dds, nw::ResourceType::tga});

            if (rdata.bytes.size()) {
                nw::Image icon{std::move(rdata)};
                QImage image(icon.release(), icon.width(), icon.height(), icon.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
                    [](void* bytes) { if (bytes) { free(bytes); } });

                // stb automatically flips standard dds and tga,
                // so here for bioware dds which is implemented a bit differently
                // needs to be flipped back to right side up.
                if (icon.is_bio_dds()) {
                    image.mirror();
                }

                item->setIcon(QIcon(QPixmap::fromImage(image)));
            }

            item->setData(val);
            model->appendRow(item);
        }
        return model;
    };

    auto result = create_model(models);
    simple_model_map.emplace(std::string(type), result);
    return result;
}
