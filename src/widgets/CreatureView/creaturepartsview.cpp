#include "creaturepartsview.h"

#include "../../services/toolset/toolsetservice.h"
#include "../util/itemmodels.h"

#include <nw/kernel/Rules.hpp>
#include <nw/rules/attributes.hpp>

#include <QStandardItemModel>
#include <QStringListModel>

QSet<int> getPartModelNumbers(std::string_view name, nw::Appearance appearance, nw::Phenotype phenotype, bool feminine)
{
    QSet<int> result;

    auto& tool = toolset();
    auto app = nw::kernel::rules().appearances.get(appearance);
    if (!app) {
        LOG_F(ERROR, "Invalid appearance");
        return result;
    }
    auto pheno = nw::kernel::rules().phenotypes.get(phenotype);
    if (!pheno) {
        LOG_F(ERROR, "Invalid phenotype");
        return result;
    }

    for (auto& part : tool.body_part_models) {
        if (part.part != name) { continue; }

        if (app->model_name.size() != 1 || app->model_name[0] != part.race) {
            continue;
        }

        if (*phenotype != part.phenotype && pheno->fallback != part.phenotype) {
            continue;
        } else if (part.female && !feminine) {
            continue;
        }

        if (result.contains(part.number)) { continue; }
        result.insert(part.number);
    }
    return result;
}

CreaturePartsView::CreaturePartsView(QWidget* parent)
    : PropertyBrowser(parent)
{
}

void CreaturePartsView::loadProperties()
{
    int row = mapSourceRowToProxyRow(toolset().appearances_model.get(), toolset().appearances_filter.get(), *obj_->appearance.id);
    auto p = makeEnumProperty("Appearance", row, toolset().appearances_filter.get());
    p->on_set = [this](const QVariant& value) {
        auto idx = toolset().appearances_filter->index(value.toInt(), 0);
        obj_->appearance.id = nw::Appearance::make(idx.data(Qt::UserRole + 1).toInt());
        emit updateModel();
    };
    addProperty(p);

    bool is_dynamic_ = nw::string::icmp(nw::kernel::rules().appearances.entries[*obj_->appearance.id].model_type, "P");

    row = mapSourceRowToProxyRow(toolset().phenotype_model.get(), toolset().phenotype_filter.get(),
        *obj_->appearance.phenotype);
    p = makeEnumProperty("Phenotype", row, toolset().phenotype_filter.get());
    p->on_set = [this](const QVariant& value) {
        auto idx = toolset().phenotype_filter->index(value.toInt(), 0);
        obj_->appearance.phenotype = nw::Phenotype::make(idx.data(Qt::UserRole + 1).toInt());
        emit updateModel();
    };
    p->setEditable(is_dynamic_);
    addProperty(p);

    auto load_part = [this](QString name, std::string_view part, uint16_t* current, Property* parent) {
        auto set = getPartModelNumbers(part, obj_->appearance.id, obj_->appearance.phenotype,
            obj_->gender == 1);

        QList<int> sorted = set.values();
        std::sort(sorted.begin(), sorted.end());

        QStringList values;
        int selected = -1;
        int i = 0;
        foreach (const auto& it, sorted) {
            values << QString::number(it);
            if (it == *current) {
                selected = i;
            }
            ++i;
        }

        if (sorted.empty()) { return; }

        if (selected == -1) {
            *current = static_cast<uint16_t>(sorted[0]);
            selected = 0;
        }

        auto prop = makeEnumProperty(name, selected, new QStringListModel(values, this), parent);
        prop->on_set = [this, current](const QVariant& value) {
            *current = static_cast<uint16_t>(value.toInt()); // This can't be right.. but no dynamic models yet.
            emit updateModel();
        };
    };

    if (is_dynamic_) {
        auto bp_grp = makeGroup("Body Parts");
        load_part("Head", "head", &obj_->appearance.body_parts.head, bp_grp);
        load_part("Neck", "neck", &obj_->appearance.body_parts.neck, bp_grp);
        load_part("Chest", "chest", &obj_->appearance.body_parts.torso, bp_grp);
        load_part("Pelvis", "pelvis", &obj_->appearance.body_parts.pelvis, bp_grp);

        load_part("Bicep, Left", "bicepl", &obj_->appearance.body_parts.bicep_left, bp_grp);
        load_part("Bicep, Right", "bicepr", &obj_->appearance.body_parts.bicep_right, bp_grp);
        load_part("Forearm, Left", "forel", &obj_->appearance.body_parts.forearm_left, bp_grp);
        load_part("Forearm, Right", "forer", &obj_->appearance.body_parts.forearm_right, bp_grp);
        load_part("Hand, Left", "handl", &obj_->appearance.body_parts.hand_left, bp_grp);
        load_part("Hand, Right", "handr", &obj_->appearance.body_parts.hand_right, bp_grp);

        load_part("Thigh, Left", "legl", &obj_->appearance.body_parts.thigh_left, bp_grp);
        load_part("Thigh, Right", "legr", &obj_->appearance.body_parts.thigh_right, bp_grp);
        load_part("Shin, Left", "shinl", &obj_->appearance.body_parts.shin_left, bp_grp);
        load_part("Shin, Right", "shinr", &obj_->appearance.body_parts.shin_right, bp_grp);
        load_part("Foot, Left", "footl", &obj_->appearance.body_parts.foot_left, bp_grp);
        load_part("Foot, Right", "footr", &obj_->appearance.body_parts.foot_right, bp_grp);

        addProperty(bp_grp);
    }

    auto acc_grp = makeGroup("Accessories");

    row = findStandardItemIndex(toolset().tails_model.get(), obj_->appearance.tail);
    p = makeEnumProperty("Tails", row, toolset().tails_model.get(), acc_grp);
    p->on_set = [this](const QVariant& value) {
        auto idx = toolset().tails_model->index(value.toInt(), 0);
        obj_->appearance.tail = idx.data(Qt::UserRole + 1).toInt();
        emit updateModel();
    };

    row = findStandardItemIndex(toolset().wings_model.get(), obj_->appearance.wings);
    p = makeEnumProperty("Wings", row, toolset().wings_model.get(), acc_grp);
    p->on_set = [this](const QVariant& value) {
        auto idx = toolset().wings_model->index(value.toInt(), 0);
        obj_->appearance.wings = idx.data(Qt::UserRole + 1).toInt();
        emit updateModel();
    };

    addProperty(acc_grp);
}

void CreaturePartsView::setCreature(nw::Creature* obj)
{
    obj_ = obj;
    loadProperties();
}
