#include "creaturepartsview.h"

#include "../arclight/toolsetservice.h"
#include "qtpropertybrowser/qtpropertybrowser.h"
#include "qtpropertybrowser/qtpropertymanager.h"

#include <nw/kernel/Rules.hpp>
#include <nw/rules/attributes.hpp>

#include <QGridLayout>

QSet<int> getPartModelNumbers(std::string_view name, nw::Appearance appearance, nw::Phenotype phenotype, bool feminine)
{
    QSet<int> result;

    auto tool = toolset();
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

        if (app->model.size() != 1 || app->model[0] != part.race) {
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
    : PropertiesView(parent)
{
    connect(bools(), &QtBoolPropertyManager::propertyChanged, this, &CreaturePartsView::onPropertyChanged);
    connect(enums(), &QtEnumPropertyManager::propertyChanged, this, &CreaturePartsView::onPropertyChanged);
    connect(ints(), &QtIntPropertyManager::propertyChanged, this, &CreaturePartsView::onPropertyChanged);
    connect(strings(), &QtStringPropertyManager::propertyChanged, this, &CreaturePartsView::onPropertyChanged);
}

void CreaturePartsView::clear()
{
    prop_func_map_.clear();
    editor()->clear();
}

void CreaturePartsView::loadProperties()
{

    auto load_part = [this](QString name, std::string_view part, uint16_t* current) {
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

        auto p = addPropertyEnum(name, selected, values);
        prop_func_map_.insert(p, [this, current](QtProperty* prop) {
            *current = static_cast<uint16_t>(enums()->name(prop).toInt());
            emit updateModel();
        });
        editor()->addProperty(p);
    };
    load_part("Head", "head", &obj_->appearance.body_parts.head);
    load_part("Neck", "neck", &obj_->appearance.body_parts.neck);
    load_part("Chest", "chest", &obj_->appearance.body_parts.torso);
    load_part("Pelvis", "pelvis", &obj_->appearance.body_parts.pelvis);

    load_part("Bicep, Left", "bicepl", &obj_->appearance.body_parts.bicep_left);
    load_part("Bicep, Right", "bicepr", &obj_->appearance.body_parts.bicep_right);
    load_part("Forearm, Left", "forel", &obj_->appearance.body_parts.forearm_left);
    load_part("Forearm, Right", "forer", &obj_->appearance.body_parts.forearm_right);
    load_part("Hand, Left", "handl", &obj_->appearance.body_parts.hand_left);
    load_part("Hand, Right", "handr", &obj_->appearance.body_parts.hand_right);

    load_part("Thigh, Left", "legl", &obj_->appearance.body_parts.thigh_left);
    load_part("Thigh, Right", "legr", &obj_->appearance.body_parts.thigh_right);
    load_part("Shin, Left", "shinl", &obj_->appearance.body_parts.shin_left);
    load_part("Shin, Right", "shinr", &obj_->appearance.body_parts.shin_right);
    load_part("Foot, Left", "footl", &obj_->appearance.body_parts.foot_left);
    load_part("Foot, Right", "footr", &obj_->appearance.body_parts.foot_right);
}

void CreaturePartsView::setCreature(nw::Creature* obj)
{
    obj_ = obj;
    loadProperties();

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(editor());
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void CreaturePartsView::setEnabled(bool enabled)
{
    for (auto p : prop_func_map_.keys()) {
        p->setEnabled(enabled);
    }
}

void CreaturePartsView::onPropertyChanged(QtProperty* prop)
{
    auto it = prop_func_map_.find(prop);
    if (it == std::end(prop_func_map_)) { return; }
    it.value()(prop);
}
