#include "creaturepropertiesview.h"

#include "../../services/toolset/toolsetservice.h"
#include "../util/itemmodels.h"
#include "../util/strings.h"

#include "nw/kernel/FactionSystem.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/objects/Creature.hpp"

#include <QStandardItemModel>

CreaturePropertiesView::CreaturePropertiesView(QWidget* parent)
    : PropertyBrowser(parent)
{
}

CreaturePropertiesView::~CreaturePropertiesView()
{
}

void CreaturePropertiesView::setCreature(nw::Creature* obj)
{
    obj_ = obj;
    loadProperties();
}

void CreaturePropertiesView::loadAbilities()
{
    Property* grp = makeGroup("Abilities");

#define ADD_ABILIITY_PROPERTY(name, loc)               \
    do {                                               \
        auto p = makeIntegerProperty(name, loc, grp);  \
        p->int_config.min = 3;                         \
        p->int_config.max = 255;                       \
        p->on_set = [this](const QVariant& value) {    \
            loc = static_cast<uint8_t>(value.toInt()); \
            emit reloadStats(); /* clazy:skip */       \
        };                                             \
    } while (0)

    ADD_ABILIITY_PROPERTY("Strength", obj_->stats.abilities_[0]);
    ADD_ABILIITY_PROPERTY("Dexterity", obj_->stats.abilities_[1]);
    ADD_ABILIITY_PROPERTY("Constituion", obj_->stats.abilities_[2]);
    ADD_ABILIITY_PROPERTY("Intelligence", obj_->stats.abilities_[3]);
    ADD_ABILIITY_PROPERTY("Wisdom", obj_->stats.abilities_[4]);
    ADD_ABILIITY_PROPERTY("Charisma", obj_->stats.abilities_[5]);

    addProperty(grp);

#undef ADD_ABILIITY_PROPERTY
}

void CreaturePropertiesView::loadBasic()
{
    Property* grp = makeGroup("Basic");

    auto p = makeStringProperty("Deity", to_qstring(obj_->deity), grp);
    p->on_set = [this](const QVariant& value) {
        obj_->deity = value.toString().toStdString();
    };

    int row = findStandardItemIndex(toolset().faction_model.get(), obj_->faction_id);
    p = makeEnumProperty("Faction", row, toolset().faction_model.get(), grp);
    p->on_set = [this](const QVariant& value) {
        auto idx = toolset().faction_model->index(value.toInt(), 0);
        obj_->faction_id = static_cast<uint16_t>(idx.data(Qt::UserRole + 1).toInt());
    };

    row = mapSourceRowToProxyRow(toolset().race_model.get(), toolset().race_filter.get(), *obj_->race);
    p = makeEnumProperty("Race", row, toolset().race_filter.get(), grp);
    p->on_set = [this](const QVariant& value) {
        int v = mapProxyRowToSourceRow(toolset().race_filter.get(), value.toInt());
        obj_->race = nw::Race::make(v);
    };

    p = makeStringProperty("Subrace", to_qstring(obj_->subrace), grp);
    p->on_set = [this](const QVariant& value) {
        obj_->subrace = value.toString().toStdString();
    };

    addProperty(grp);
}

void CreaturePropertiesView::loadInterface()
{
    Property* grp = makeGroup("Interface");

    auto p = makeIntegerProperty("Corpse Decay Time", obj_->decay_time / 1000, grp);
    p->int_config.min = 0;
    p->int_config.min = INT_MAX;
    p->on_set = [this](const QVariant& value) {
        obj_->decay_time = value.toInt() * 1000; // Actual value is in ms
    };

    p = makeBoolProperty("Disarmable", obj_->disarmable, grp);
    p->on_set = [this](const QVariant& value) {
        obj_->disarmable = value.toBool();
    };

    p = makeBoolProperty("Immortal", obj_->immortal, grp);
    p->on_set = [this](const QVariant& value) {
        obj_->immortal = value.toBool();
    };

    p = makeBoolProperty("Lootable Corpse", obj_->lootable, grp);
    p->on_set = [this](const QVariant& value) {
        obj_->lootable = value.toBool();
    };

    p = makeBoolProperty("No Permanent Death", obj_->chunk_death, grp);
    p->on_set = [this](const QVariant& value) {
        obj_->chunk_death = value.toBool();
    };

    p = makeBoolProperty("Plot", obj_->plot, grp);
    p->on_set = [this](const QVariant& value) {
        obj_->plot = value.toBool();
    };

    addProperty(grp);
}

void CreaturePropertiesView::loadSaves()
{
    Property* grp = makeGroup("Save Bonus");

    auto p = makeIntegerProperty("Fortitude", obj_->stats.save_bonus.fort, grp);
    p->int_config.min = 0;
    p->int_config.max = 255;
    p->on_set = [this](const QVariant& value) {
        obj_->stats.save_bonus.fort = static_cast<int16_t>(value.toInt());
        emit reloadStats();
    };

    p = makeIntegerProperty("Reflex", obj_->stats.save_bonus.reflex, grp);
    p->int_config.min = 0;
    p->int_config.max = 255;
    p->on_set = [this](const QVariant& value) {
        obj_->stats.save_bonus.reflex = static_cast<int16_t>(value.toInt());
        emit reloadStats();
    };

    p = makeIntegerProperty("Will", obj_->stats.save_bonus.will, grp);
    p->int_config.min = 0;
    p->int_config.max = 255;
    p->on_set = [this](const QVariant& value) {
        obj_->stats.save_bonus.will = static_cast<int16_t>(value.toInt());
        emit reloadStats();
    };

    addProperty(grp);
}

void CreaturePropertiesView::loadScripts()
{
    Property* grp = makeGroup("Scripts");

#define ADD_SCRIPT(name, resref)                                                         \
    do {                                                                                 \
        auto p = makeStringProperty(name, to_qstring(obj_->scripts.resref.view()), grp); \
        p->on_set = [this](const QVariant& value) {                                      \
            obj_->scripts.resref = nw::Resref{value.toString().toStdString()};           \
        };                                                                               \
    } while (0)

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

#undef ADD_SCRIPT

    addProperty(grp);
}

void CreaturePropertiesView::loadSkills()
{
    Property* grp = makeGroup("Skills");

    int i = 0;
    for (const auto& skill : nw::kernel::rules().skills.entries) {
        if (skill.valid()) {
            auto p = makeIntegerProperty(to_qstring(nw::kernel::strings().get(skill.name)), obj_->stats.skills_[i], grp);
            p->int_config.min = 0;
            p->int_config.max = 255;
            p->on_set = [this, i](const QVariant& value) {
                obj_->stats.skills_[i] = static_cast<uint8_t>(value.toInt());
                emit reloadStats();
            };
        }
        ++i;
    }

    std::sort(grp->children.begin(), grp->children.end(), [](auto lhs, auto rhs) {
        return lhs->name < rhs->name;
    });

    addProperty(grp);
}

void CreaturePropertiesView::loadAdvanced()
{
    Property* grp = makeGroup("Advanced");

    int row = findStandardItemIndex(toolset().creaturespeed_model.get(), obj_->walkrate);
    auto p = makeEnumProperty("Movement Rate", row, toolset().creaturespeed_model.get(), grp);
    p->on_set = [this](const QVariant& value) {
        auto idx = toolset().creaturespeed_model->index(value.toInt(), 0);
        obj_->walkrate = static_cast<uint8_t>(idx.data(Qt::UserRole + 1).toInt());
    };

    row = findStandardItemIndex(toolset().ranges_model.get(), obj_->perception_range);
    p = makeEnumProperty("Perception Range", row, toolset().ranges_model.get(), grp);
    p->on_set = [this](const QVariant& value) {
        auto idx = toolset().ranges_model->index(value.toInt(), 0);
        obj_->walkrate = static_cast<uint8_t>(idx.data(Qt::UserRole + 1).toInt());
    };

    addProperty(grp);
}

void CreaturePropertiesView::loadProperties()
{
    loadBasic();
    loadAbilities();
    loadSaves();
    loadScripts();
    loadSkills();
    loadInterface();
    loadAdvanced();
}
