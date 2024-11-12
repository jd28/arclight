#include "creaturepropertiesview.h"

#include "nw/objects/Creature.hpp"

#include "qtpropertybrowser/qtpropertybrowser.h"
#include "qtpropertybrowser/qtpropertymanager.h"
#include "util/strings.h"

#include <QGridLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <nw/kernel/Rules.hpp>
#include <nw/kernel/TwoDACache.hpp>

CreaturePropertiesView::CreaturePropertiesView(QWidget* parent)
    : PropertiesView(parent)
{
    connect(bools(), &QtBoolPropertyManager::propertyChanged, this, &CreaturePropertiesView::onPropertyChanged);
    connect(enums(), &QtEnumPropertyManager::propertyChanged, this, &CreaturePropertiesView::onPropertyChanged);
    connect(ints(), &QtIntPropertyManager::propertyChanged, this, &CreaturePropertiesView::onPropertyChanged);
    connect(strings(), &QtStringPropertyManager::propertyChanged, this, &CreaturePropertiesView::onPropertyChanged);
}

CreaturePropertiesView::~CreaturePropertiesView()
{
}

void CreaturePropertiesView::setCreature(nw::Creature* obj)
{
    obj_ = obj;
    loadProperties();

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(editor());
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void CreaturePropertiesView::onPropertyChanged(QtProperty* prop)
{
    auto it = prop_func_map_.find(prop);
    if (it == std::end(prop_func_map_)) { return; }
    it.value()(prop);
}

void CreaturePropertiesView::loadAbilities()
{
    QtProperty* grp_abilities = addGroup("Abilities");
    QList<QtProperty*> abilities;

    abilities << addPropertyInt("Strength", obj_->stats.abilities_[0], 3, 255);
    prop_func_map_.insert(abilities.back(), [this](QtProperty* prop) {
        obj_->stats.abilities_[0] = static_cast<uint8_t>(ints()->value(prop));
        emit updateStats();
    });

    abilities << addPropertyInt("Dexterity", obj_->stats.abilities_[1], 3, 255);
    prop_func_map_.insert(abilities.back(), [this](QtProperty* prop) {
        obj_->stats.abilities_[1] = static_cast<uint8_t>(ints()->value(prop));
        emit updateStats();
    });

    abilities << addPropertyInt("Constituion", obj_->stats.abilities_[2], 3, 255);
    prop_func_map_.insert(abilities.back(), [this](QtProperty* prop) {
        obj_->stats.abilities_[2] = static_cast<uint8_t>(ints()->value(prop));
        emit updateStats();
    });

    abilities << addPropertyInt("Constituion", obj_->stats.abilities_[2], 3, 255);
    prop_func_map_.insert(abilities.back(), [this](QtProperty* prop) {
        obj_->stats.abilities_[2] = static_cast<uint8_t>(ints()->value(prop));
        emit updateStats();
    });

    abilities << addPropertyInt("Intelligence", obj_->stats.abilities_[3], 3, 255);
    prop_func_map_.insert(abilities.back(), [this](QtProperty* prop) {
        obj_->stats.abilities_[3] = static_cast<uint8_t>(ints()->value(prop));
        emit updateStats();
    });

    abilities << addPropertyInt("Wisdom", obj_->stats.abilities_[4], 3, 255);
    prop_func_map_.insert(abilities.back(), [this](QtProperty* prop) {
        obj_->stats.abilities_[4] = static_cast<uint8_t>(ints()->value(prop));
        emit updateStats();
    });

    abilities << addPropertyInt("Charisma", obj_->stats.abilities_[5], 3, 255);
    prop_func_map_.insert(abilities.back(), [this](QtProperty* prop) {
        obj_->stats.abilities_[5] = static_cast<uint8_t>(ints()->value(prop));
        emit updateStats();
    });

    foreach (auto& s, abilities) {
        grp_abilities->addSubProperty(s);
    }

    editor()->addProperty(grp_abilities);
}

void CreaturePropertiesView::loadSaves()
{
    QtProperty* grp_saves = addGroup("Save Bonus");
    QList<QtProperty*> saves;

    saves << addPropertyInt("Fortitude", obj_->stats.save_bonus.fort, 0, 255);
    prop_func_map_.insert(saves.back(), [this](QtProperty* prop) {
        obj_->stats.save_bonus.fort = static_cast<int16_t>(ints()->value(prop));
        emit updateStats();
    });

    saves << addPropertyInt("Reflex", obj_->stats.save_bonus.reflex, 0, 255);
    prop_func_map_.insert(saves.back(), [this](QtProperty* prop) {
        obj_->stats.save_bonus.reflex = static_cast<int16_t>(ints()->value(prop));
        emit updateStats();
    });

    saves << addPropertyInt("Will", obj_->stats.save_bonus.will, 0, 255);
    prop_func_map_.insert(saves.back(), [this](QtProperty* prop) {
        obj_->stats.save_bonus.will = static_cast<int16_t>(ints()->value(prop));
        emit updateStats();
    });

    foreach (auto& s, saves) {
        grp_saves->addSubProperty(s);
    }

    editor()->addProperty(grp_saves);
}

void CreaturePropertiesView::loadScripts()
{
    QtProperty* grp = addGroup("Scripts");
    QList<QtProperty*> entries;

#define ADD_SCRIPT(name, resref)                                                 \
    entries << addPropertyString(name, to_qstring(obj_->scripts.resref.view())); \
    prop_func_map_.insert(entries.back(), [this](QtProperty* prop) {             \
        obj_->scripts.resref = nw::Resref(strings()->value(prop).toStdString()); \
    })

    ADD_SCRIPT("On Blocked", on_blocked);
    ADD_SCRIPT("On Combat End Round", on_endround);
    ADD_SCRIPT("On Conversation", on_conversation);
    ADD_SCRIPT("On Damaged", on_damaged);
    ADD_SCRIPT("On Death", on_death);
    ADD_SCRIPT("On Disturbed", on_disturbed);
    ADD_SCRIPT("On Heartbeat", on_heartbeat);
    ADD_SCRIPT("On Perception", on_perceived);
    ADD_SCRIPT("On Physically Attacked", on_attacked);
    ADD_SCRIPT("On Rested", on_rested);
    ADD_SCRIPT("On Spawn", on_spawn);
    ADD_SCRIPT("On Spell Cast At", on_spell_cast_at);
    ADD_SCRIPT("On User Defined", on_user_defined);

    foreach (auto& e, entries) {
        grp->addSubProperty(e);
    }

    editor()->addProperty(grp);

#undef ADD_SCRIPT
}

void CreaturePropertiesView::loadSkills()
{
    QtProperty* grp_skills = addGroup("Skills");
    QList<QtProperty*> skills;

    int i = 0;
    for (const auto& skill : nw::kernel::rules().skills.entries) {
        if (skill.valid()) {
            skills << addPropertyInt(to_qstring(nw::kernel::strings().get(skill.name)), obj_->stats.skills_[i], 0, 255);
            prop_func_map_.insert(skills.back(), [this, i](QtProperty* prop) {
                obj_->stats.skills_[i] = static_cast<uint8_t>(ints()->value(prop));
                emit updateStats();
            });
        }
        ++i;
    }

    std::sort(skills.begin(), skills.end(), [](auto lhs, auto rhs) {
        return lhs->propertyName() < rhs->propertyName();
    });

    foreach (auto& s, skills) {
        grp_skills->addSubProperty(s);
    }

    editor()->addProperty(grp_skills);
}

void CreaturePropertiesView::loadAdvanced()
{
    QtProperty* grp = addGroup("Advanced");
    QList<QtProperty*> entries;

    // Movement rate
    QList<QPair<QString, int>> moverates;
    auto creaturespeed_2da = nw::kernel::twodas().get("creaturespeed");
    if (creaturespeed_2da) {
        for (size_t i = 0; i < creaturespeed_2da->rows(); ++i) {
            int name;
            if (creaturespeed_2da->get_to(i, "Name", name)) {
                auto string = nw::kernel::strings().get(uint32_t(name));
                moverates << QPair<QString, int>{to_qstring(string), int(i)};
            }
        }
    }

    QStringList names;
    QList<QVariant> data;

    std::sort(moverates.begin(), moverates.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first < rhs.first;
    });

    int creaturespeed_idx = -1;
    int i = 0;
    foreach (const auto& r, moverates) {
        names << r.first;
        data << r.second;
        if (r.second == obj_->walkrate) {
            creaturespeed_idx = i;
        }
        ++i;
    }

    entries << addPropertyEnum("Movement Rate", creaturespeed_idx, names, data);
    prop_func_map_.insert(entries.back(), [this](QtProperty* prop) {
        obj_->walkrate = static_cast<uint8_t>(enums()->data(prop).toInt());
    });

    // Perception
    QList<QPair<QString, int>> ranges;
    auto ranges_2da = nw::kernel::twodas().get("ranges");
    if (ranges_2da) {
        for (size_t j = 0; j < ranges_2da->rows(); ++j) {
            int name;
            if (ranges_2da->get_to(j, "Name", name)) {
                auto string = nw::kernel::strings().get(uint32_t(name));
                ranges << QPair<QString, int>{to_qstring(string), int(j)};
            }
        }
    }

    std::sort(ranges.begin(), ranges.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first < rhs.first;
    });

    names.clear();
    data.clear();

    i = 0;
    int ranges_idx = -1;
    foreach (const auto& r, ranges) {
        names << r.first;
        data << r.second;
        if (r.second == obj_->perception_range) {
            ranges_idx = i;
        }
        ++i;
    }

    entries << addPropertyEnum("Perception Range", ranges_idx, names, data);
    prop_func_map_.insert(entries.back(), [this](QtProperty* prop) {
        obj_->perception_range = static_cast<uint8_t>(enums()->data(prop).toInt());
    });

    foreach (auto& e, entries) {
        grp->addSubProperty(e);
    }

    editor()->addProperty(grp);
}

void CreaturePropertiesView::loadProperties()
{
    QtProperty* grp_basic = addGroup("Basic");
    QList<QtProperty*> basic;

    basic << addPropertyString("Subrace", to_qstring(obj_->subrace));
    prop_func_map_.insert(basic.back(), [this](QtProperty* p) {
        obj_->common.tag = nw::kernel::strings().intern(strings()->value(p).toStdString());
    });

    basic << addPropertyString("Tag", to_qstring(obj_->common.tag.view()));
    prop_func_map_.insert(basic.back(), [this](QtProperty* p) {
        obj_->common.tag = nw::kernel::strings().intern(strings()->value(p).toStdString());
    });

    basic << addPropertyString("Template Resref", to_qstring(obj_->common.resref.view()));
    basic.back()->setEnabled(false);

    foreach (auto& s, basic) {
        grp_basic->addSubProperty(s);
    }
    editor()->addProperty(grp_basic);

    loadAbilities();
    loadSaves();
    loadScripts();
    loadSkills();
    loadAdvanced();
}
