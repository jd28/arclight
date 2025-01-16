#include "placeableproperties.h"

#include "../qtpropertybrowser/qteditorfactory.h"
#include "../qtpropertybrowser/qtpropertybrowser.h"
#include "../qtpropertybrowser/qtpropertymanager.h"
#include "../util/strings.h"

#include "nw/kernel/FactionSystem.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/objects/Placeable.hpp"

#include <QCompleter>
#include <QGridLayout>

#include <limits>

static QRegularExpression resref_regex("^[a-z_]{1,16}$");

PlaceableProperties::PlaceableProperties(QWidget* parent)
    : PropertiesView{parent}
{
    connect(bools(), &QtBoolPropertyManager::propertyChanged, this, &PlaceableProperties::onPropertyChanged);
    connect(enums(), &QtEnumPropertyManager::propertyChanged, this, &PlaceableProperties::onPropertyChanged);
    connect(ints(), &QtIntPropertyManager::propertyChanged, this, &PlaceableProperties::onPropertyChanged);
    connect(strings(), &QtStringPropertyManager::propertyChanged, this, &PlaceableProperties::onPropertyChanged);
}

void PlaceableProperties::savesLoad()
{
    QtProperty* grp_saves = addGroup("Saves");
    QList<QtProperty*> saves;
    saves << addPropertyInt("Fortitude", obj_->saves.fort, 0, 255);
    prop_func_map_.insert(saves.back(), [this](QtProperty* prop) {
        obj_->saves.fort = static_cast<int16_t>(ints()->value(prop));
    });
    saves << addPropertyInt("Reflex", obj_->saves.reflex, 0, 255);
    prop_func_map_.insert(saves.back(), [this](QtProperty* prop) {
        obj_->saves.reflex = static_cast<int16_t>(ints()->value(prop));
    });
    saves << addPropertyInt("Will", obj_->saves.will, 0, 255);
    prop_func_map_.insert(saves.back(), [this](QtProperty* prop) {
        obj_->saves.will = static_cast<int16_t>(ints()->value(prop));
    });
    for (auto s : saves) {
        grp_saves->addSubProperty(s);
    }
    editor()->addProperty(grp_saves);
}

void PlaceableProperties::setObject(nw::Placeable* obj)
{
    if (obj_) { return; }
    obj_ = obj;

    basicsLoad();
    conversationLoad();
    locksLoad();
    savesLoad();
    scriptsLoad();
    trapsLoad();

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(editor());
    setLayout(layout);
}

void PlaceableProperties::onPropertyChanged(QtProperty* prop)
{
    auto it = prop_func_map_.find(prop);
    if (it == std::end(prop_func_map_)) { return; }
    it.value()(prop);
}

// == Private Methods =========================================================
// ============================================================================

void PlaceableProperties::basicsLoad()
{
    QtProperty* grp_basic = addGroup("Basic");
    QStringList qfactions;
    for (const auto& fac : nw::kernel::factions().all()) {
        qfactions << to_qstring(fac);
    }
    auto fac = addPropertyEnum("Faction", obj_->faction, std::move(qfactions));
    prop_func_map_.insert(fac, [this](QtProperty* prop) {
        obj_->hardness = static_cast<uint8_t>(enums()->value(prop));
    });
    grp_basic->addSubProperty(fac);

    auto hardness = addPropertyInt("Hardness", obj_->hardness, 0, 255);
    prop_func_map_.insert(hardness, [this](QtProperty* prop) {
        obj_->hardness = static_cast<uint8_t>(ints()->value(prop));
    });
    grp_basic->addSubProperty(hardness);

    auto has_inventory = addPropertyBool("Has Inventory", obj_->has_inventory);
    prop_func_map_.insert(has_inventory, [this](QtProperty* prop) {
        obj_->has_inventory = bools()->value(prop);
    });
    grp_basic->addSubProperty(has_inventory);

    auto hitpoints = addPropertyInt("Hitpoints", obj_->hp, 0, std::numeric_limits<int16_t>::max());
    prop_func_map_.insert(hitpoints, [this](QtProperty* prop) {
        obj_->hp = static_cast<uint8_t>(ints()->value(prop));
    });
    grp_basic->addSubProperty(hitpoints);

    auto plot = addPropertyBool("Plot", obj_->plot);
    prop_func_map_.insert(plot, [this](QtProperty* prop) {
        obj_->plot = bools()->value(prop);
    });
    grp_basic->addSubProperty(plot);

    auto static_ = addPropertyBool("Static", obj_->static_);
    prop_func_map_.insert(static_, [this](QtProperty* prop) {
        obj_->static_ = bools()->value(prop);
        locksUpdate();
        trapsUpdate();
    });
    grp_basic->addSubProperty(static_);

    auto useable = addPropertyBool("Useable", obj_->useable);
    prop_func_map_.insert(useable, [this](QtProperty* prop) {
        obj_->useable = bools()->value(prop);
        locksUpdate();
        trapsUpdate();
    });
    grp_basic->addSubProperty(useable);

    editor()->addProperty(grp_basic);
}

void PlaceableProperties::conversationLoad()
{
    QtProperty* grp_conv = addGroup("Conversation");
    auto prop = addPropertyString("Dialog", obj_->conversation, resref_regex);
    prop_func_map_.insert(prop, [this](QtProperty* p) {
        obj_->conversation = nw::Resref{strings()->value(p).toStdString()};
    });
    grp_conv->addSubProperty(prop);
    prop = addPropertyBool("Interruptable", obj_->interruptable);
    prop_func_map_.insert(prop, [this](QtProperty* p) {
        obj_->interruptable = bools()->value(p);
    });
    grp_conv->addSubProperty(prop);
    editor()->addProperty(grp_conv);
}

void PlaceableProperties::locksLoad()
{
    QtProperty* grp_lock = addGroup("Lock");

    lock_locked_prop_ = addPropertyBool("Locked", obj_->lock.locked);
    prop_func_map_.insert(lock_locked_prop_, [this](QtProperty* prop) {
        LOG_F(INFO, "locked: {}", bools()->value(prop));
        obj_->lock.locked = bools()->value(prop);
        locksUpdate();
    });
    grp_lock->addSubProperty(lock_locked_prop_);

    lock_lockable_prop_ = addPropertyBool("Relockable", obj_->lock.lockable);
    prop_func_map_.insert(lock_lockable_prop_, [this](QtProperty* prop) {
        obj_->lock.lockable = bools()->value(prop);
        locksUpdate();
    });
    grp_lock->addSubProperty(lock_lockable_prop_);

    lock_remove_key_prop_ = addPropertyBool("Remove Key", obj_->lock.remove_key);
    prop_func_map_.insert(lock_remove_key_prop_, [this](QtProperty* prop) {
        obj_->lock.remove_key = bools()->value(prop);
        locksUpdate();
    });
    grp_lock->addSubProperty(lock_remove_key_prop_);

    lock_key_required_prop_ = addPropertyBool("Key Required", obj_->lock.key_required);
    prop_func_map_.insert(lock_key_required_prop_, [this](QtProperty* prop) {
        obj_->lock.key_required = bools()->value(prop);
        locksUpdate();
    });
    grp_lock->addSubProperty(lock_key_required_prop_);

    lock_key_name_prop_ = addPropertyString("Key Tag", obj_->lock.key_name);
    prop_func_map_.insert(lock_key_name_prop_, [this](QtProperty* prop) {
        obj_->lock.key_name = strings()->value(prop).toStdString();
        locksUpdate();
    });
    grp_lock->addSubProperty(lock_key_name_prop_);

    lock_lock_dc_prop_ = addPropertyInt("Lock DC", obj_->lock.lock_dc, 0, 255);
    prop_func_map_.insert(lock_lock_dc_prop_, [this](QtProperty* prop) {
        obj_->lock.lock_dc = static_cast<uint8_t>(ints()->value(prop));
        locksUpdate();
    });
    grp_lock->addSubProperty(lock_lock_dc_prop_);

    lock_unlock_dc_prop_ = addPropertyInt("Unlock DC", obj_->lock.unlock_dc, 0, 255);
    prop_func_map_.insert(lock_unlock_dc_prop_, [this](QtProperty* prop) {
        obj_->lock.unlock_dc = static_cast<uint8_t>(ints()->value(prop));
        locksUpdate();
    });
    grp_lock->addSubProperty(lock_unlock_dc_prop_);
    locksUpdate();
    editor()->addProperty(grp_lock);
}

void PlaceableProperties::locksUpdate()
{
    lock_locked_prop_->setEnabled(obj_->useable);
    lock_lockable_prop_->setEnabled(obj_->useable);
    lock_remove_key_prop_->setEnabled((obj_->lock.locked || obj_->lock.lockable) && obj_->lock.key_required);
    lock_key_required_prop_->setEnabled(!obj_->lock.key_name.empty());
    lock_key_name_prop_->setEnabled(obj_->lock.locked || obj_->lock.lockable);
    lock_lock_dc_prop_->setEnabled(obj_->lock.lockable && !obj_->lock.key_required);
    lock_unlock_dc_prop_->setEnabled((obj_->lock.locked || obj_->lock.lockable) && !obj_->lock.key_required);
}

void PlaceableProperties::scriptsLoad()
{
    QtProperty* grp_script = addGroup("Scripts");
    QList<QtProperty*> scripts;
    scripts << addPropertyString("On Clicked", obj_->scripts.on_click, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_click = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Closed", obj_->scripts.on_closed, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_closed = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Damaged", obj_->scripts.on_damaged, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_damaged = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Death", obj_->scripts.on_death, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_death = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Trap Disarmed", obj_->scripts.on_disarm, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_disarm = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Heartbeat", obj_->scripts.on_heartbeat, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_heartbeat = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Inventory Disturbed", obj_->scripts.on_inventory_disturbed, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_inventory_disturbed = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Locked", obj_->scripts.on_lock, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_lock = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Melee Attacked", obj_->scripts.on_melee_attacked, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_melee_attacked = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Open", obj_->scripts.on_open, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_open = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Spell Cast At", obj_->scripts.on_spell_cast_at, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_spell_cast_at = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Trap Triggered", obj_->scripts.on_trap_triggered, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_trap_triggered = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Unlock", obj_->scripts.on_unlock, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_unlock = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On Used", obj_->scripts.on_used, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_used = nw::Resref{strings()->value(prop).toStdString()};
    });
    scripts << addPropertyString("On User Defined", obj_->scripts.on_user_defined, resref_regex);
    prop_func_map_.insert(scripts.back(), [this](QtProperty* prop) {
        obj_->scripts.on_user_defined = nw::Resref{strings()->value(prop).toStdString()};
    });

    for (auto s : scripts) {
        grp_script->addSubProperty(s);
    }
    editor()->addProperty(grp_script);
}
void PlaceableProperties::trapsLoad()
{
    QtProperty* grp_trap = addGroup("Trap");

    trap_is_trapped_ = addPropertyBool("Trapped", obj_->trap.is_trapped);
    prop_func_map_.insert(trap_is_trapped_, [this](QtProperty* prop) {
        obj_->trap.is_trapped = bools()->value(prop);
        trapsUpdate();
    });
    grp_trap->addSubProperty(trap_is_trapped_);

    QStringList trap_names;
    QList<QVariant> trap_data;
    auto& traps = nw::kernel::rules().traps.entries;
    for (size_t i = 0; i < traps.size(); ++i) {
        if (!traps[i].valid()) { continue; }
        trap_names << to_qstring(nw::kernel::strings().get(traps[i].name));
        trap_data << int(i);
    }
    trap_type_ = addPropertyEnum("Type", *obj_->trap.type, trap_names, trap_data);
    prop_func_map_.insert(trap_type_, [this](QtProperty* prop) {
        int value = enums()->data(prop).toInt();
        obj_->trap.type = nw::TrapType::make(value);
    });

    grp_trap->addSubProperty(trap_type_);

    trap_detectable_ = addPropertyBool("Detectable", obj_->trap.detectable);
    prop_func_map_.insert(trap_detectable_, [this](QtProperty* prop) {
        obj_->trap.detectable = bools()->value(prop);
        trapsUpdate();
    });
    grp_trap->addSubProperty(trap_detectable_);

    trap_detect_dc_ = addPropertyInt("Detection DC", obj_->trap.detect_dc, 0, 255);
    prop_func_map_.insert(trap_detect_dc_, [this](QtProperty* prop) {
        obj_->trap.detect_dc = static_cast<uint8_t>(ints()->value(prop));
        trapsUpdate();
    });
    grp_trap->addSubProperty(trap_detect_dc_);

    trap_disarmable_ = addPropertyBool("Disarmable", obj_->trap.disarmable);
    prop_func_map_.insert(trap_disarmable_, [this](QtProperty* prop) {
        obj_->trap.disarmable = bools()->value(prop);
        trapsUpdate();
    });
    grp_trap->addSubProperty(trap_disarmable_);

    trap_disarm_dc_ = addPropertyInt("Disarm DC", obj_->trap.disarm_dc, 0, 255);
    prop_func_map_.insert(trap_detect_dc_, [this](QtProperty* prop) {
        obj_->trap.disarm_dc = static_cast<uint8_t>(ints()->value(prop));
        trapsUpdate();
    });
    grp_trap->addSubProperty(trap_disarm_dc_);

    trap_one_shot_ = addPropertyBool("One Shot", obj_->trap.one_shot);
    prop_func_map_.insert(trap_one_shot_, [this](QtProperty* prop) {
        obj_->trap.one_shot = bools()->value(prop);
        trapsUpdate();
    });
    grp_trap->addSubProperty(trap_one_shot_);

    trapsUpdate();
    editor()->addProperty(grp_trap);
}

void PlaceableProperties::trapsUpdate()
{
    trap_type_->setEnabled(obj_->trap.is_trapped);
    trap_is_trapped_->setEnabled(obj_->useable);
    trap_detectable_->setEnabled(obj_->trap.is_trapped);
    trap_detect_dc_->setEnabled(obj_->trap.detectable);
    trap_disarmable_->setEnabled(obj_->trap.is_trapped);
    trap_disarm_dc_->setEnabled(obj_->trap.disarmable);
    trap_one_shot_->setEnabled(obj_->trap.is_trapped);
}
