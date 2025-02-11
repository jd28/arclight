#include "doorproperties.h"

#include "../../services/toolsetservice.h"
#include "../util/itemmodels.h"
#include "../util/strings.h"

#include "nw/kernel/FactionSystem.hpp"
#include "nw/objects/Door.hpp"

#include <QCompleter>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QStringListModel>

#include <limits>

static QRegularExpression resref_regex("^[a-z_]{0,16}$");

DoorProperties::DoorProperties(QWidget* parent)
    : PropertyBrowser{parent}
{
}

void DoorProperties::setObject(nw::Door* obj)
{
    if (obj_) { return; }
    obj_ = obj;

    appearanceLoad();
    basicsLoad();
    conversationLoad();
    locksLoad();
    savesLoad();
    scriptsLoad();
    transitionLoad();
    trapsLoad();
}

void DoorProperties::appearanceLoad()
{
    Property* grp_app = makeGroup("Appearance");

    int row = mapSourceRowToProxyRow(toolset().doortypes_model.get(), toolset().doortypes_filter.get(), obj_->appearance);
    auto doortype = makeEnumProperty("Door Type", row, toolset().doortypes_filter.get(), grp_app);
    doortype->on_set = [this](const QVariant& index) {
        int value = mapProxyRowToSourceRow(toolset().doortypes_filter.get(), index.toInt());
        obj_->appearance = value;
        generic_prop_->setEditable(obj_->appearance == 0);
        model()->updateReadOnly(generic_prop_);
        emit appearanceChanged();
    };

    row = mapSourceRowToProxyRow(toolset().genericdoors_model.get(), toolset().genericdoors_filter.get(), obj_->generic_type);
    generic_prop_ = makeEnumProperty("Generic Door Type", row, toolset().genericdoors_filter.get(), grp_app);
    generic_prop_->setEditable(obj_->appearance == 0);
    generic_prop_->on_set = [this](const QVariant& index) {
        int value = mapProxyRowToSourceRow(toolset().genericdoors_filter.get(), index.toInt());
        obj_->generic_type = value;
        emit appearanceChanged();
    };

    addProperty(grp_app);
}

// == Private Methods =========================================================
// ============================================================================

void DoorProperties::basicsLoad()
{
    Property* grp_basic = makeGroup("Basic");

    auto model = toolset().faction_model.get();
    auto index = findStandardItemIndex(model, int(obj_->faction));
    auto fac = makeEnumProperty("Faction", index, model, grp_basic);
    fac->on_set = [this, model](const QVariant& value) {
        auto idx = model->index(value.toInt(), 0);
        int mv = model->data(idx, Qt::UserRole + 1).toInt();
        obj_->faction = static_cast<uint32_t>(mv);
    };

    auto hardness = makeIntegerProperty("Hardness", obj_->hardness, grp_basic);
    hardness->int_config.min = 0;
    hardness->int_config.max = 255;
    hardness->on_set = [this](const QVariant& value) {
        obj_->hardness = static_cast<uint8_t>(value.toInt());
    };

    auto hitpoints = makeIntegerProperty("Hitpoints", obj_->hp, grp_basic);
    hitpoints->int_config.min = 0;
    hitpoints->int_config.max = std::numeric_limits<int16_t>::max();
    hitpoints->on_set = [this](const QVariant& value) {
        obj_->hp = static_cast<int16_t>(value.toInt());
    };

    auto plot = makeBoolProperty("Plot", obj_->plot, grp_basic);
    plot->on_set = [this](const QVariant& value) {
        obj_->plot = value.toBool();
    };

    addProperty(grp_basic);
}

void DoorProperties::conversationLoad()
{
    auto grp_conv = makeGroup("Conversation");
    auto prop = makeStringProperty("Dialog", to_qstring(obj_->conversation.view()), grp_conv);
    prop->on_set = [this](const QVariant& value) {
        obj_->conversation = nw::Resref{value.toString().toStdString()};
    };

    prop = makeBoolProperty("Interruptable", obj_->interruptable, grp_conv);
    prop->on_set = [this](const QVariant& value) {
        obj_->interruptable = value.toBool();
    };

    addProperty(grp_conv);
}

void DoorProperties::locksLoad()
{
    Property* grp_lock = makeGroup("Lock");

    lock_locked_prop_ = makeBoolProperty("Locked", obj_->lock.locked, grp_lock);
    lock_locked_prop_->on_set = [this](const QVariant& value) {
        obj_->lock.locked = value.toBool();
        locksUpdate();
    };

    lock_lockable_prop_ = makeBoolProperty("Relockable", obj_->lock.lockable, grp_lock);
    lock_lockable_prop_->on_set = [this](const QVariant& value) {
        obj_->lock.lockable = value.toBool();
        locksUpdate();
    };

    lock_remove_key_prop_ = makeBoolProperty("Remove Key", obj_->lock.remove_key, grp_lock);
    lock_remove_key_prop_->on_set = [this](const QVariant& value) {
        obj_->lock.remove_key = value.toBool();
        locksUpdate();
    };

    lock_key_required_prop_ = makeBoolProperty("Key Required", obj_->lock.key_required, grp_lock);
    lock_key_required_prop_->on_set = [this](const QVariant& value) {
        obj_->lock.key_required = value.toBool();
        locksUpdate();
    };

    lock_key_name_prop_ = makeStringProperty("Key Tag", to_qstring(obj_->lock.key_name), grp_lock);
    lock_key_name_prop_->on_set = [this](const QVariant& value) {
        obj_->lock.key_name = value.toString().toStdString();
        locksUpdate();
    };

    lock_lock_dc_prop_ = makeIntegerProperty("Lock DC", obj_->lock.lock_dc, grp_lock);
    lock_lock_dc_prop_->int_config.min = 0;
    lock_lock_dc_prop_->int_config.max = 255;
    lock_lock_dc_prop_->on_set = [this](const QVariant& value) {
        obj_->lock.lock_dc = static_cast<uint8_t>(value.toInt());
        locksUpdate();
    };

    lock_unlock_dc_prop_ = makeIntegerProperty("Unlock DC", obj_->lock.unlock_dc, grp_lock);
    lock_unlock_dc_prop_->int_config.min = 0;
    lock_unlock_dc_prop_->int_config.max = 255;
    lock_unlock_dc_prop_->on_set = [this](const QVariant& value) {
        obj_->lock.unlock_dc = static_cast<uint8_t>(value.toInt());
        locksUpdate();
    };

    locksUpdate();

    addProperty(grp_lock);
}

void DoorProperties::locksUpdate()
{
    lock_locked_prop_->setEditable(true);
    model()->updateReadOnly(lock_locked_prop_);
    lock_lockable_prop_->setEditable(true);
    model()->updateReadOnly(lock_lockable_prop_);
    lock_remove_key_prop_->setEditable((obj_->lock.locked || obj_->lock.lockable) && obj_->lock.key_required);
    model()->updateReadOnly(lock_remove_key_prop_);
    lock_key_required_prop_->setEditable(!obj_->lock.key_name.empty());
    model()->updateReadOnly(lock_key_required_prop_);
    lock_key_name_prop_->setEditable(obj_->lock.locked || obj_->lock.lockable);
    model()->updateReadOnly(lock_key_name_prop_);
    lock_lock_dc_prop_->setEditable(obj_->lock.lockable && !obj_->lock.key_required);
    model()->updateReadOnly(lock_lock_dc_prop_);
    lock_unlock_dc_prop_->setEditable((obj_->lock.locked || obj_->lock.lockable) && !obj_->lock.key_required);
    model()->updateReadOnly(lock_unlock_dc_prop_);
}

void DoorProperties::savesLoad()
{
    Property* grp_saves = makeGroup("Saves");

    auto save = makeIntegerProperty("Fortitude", obj_->saves.fort, grp_saves);
    save->int_config.min = 0;
    save->int_config.max = 255;
    save->on_set = [this](const QVariant& value) {
        obj_->saves.fort = static_cast<int16_t>(value.toInt());
    };

    save = makeIntegerProperty("Fortitude", obj_->saves.fort, grp_saves);
    save->int_config.min = 0;
    save->int_config.max = 255;
    save->on_set = [this](const QVariant& value) {
        obj_->saves.reflex = static_cast<int16_t>(value.toInt());
    };

    save = makeIntegerProperty("Fortitude", obj_->saves.fort, grp_saves);
    save->int_config.min = 0;
    save->int_config.max = 255;
    save->on_set = [this](const QVariant& value) {
        obj_->saves.will = static_cast<int16_t>(value.toInt());
    };
}

void DoorProperties::scriptsLoad()
{
    auto grp_script = makeGroup("Scripts");

#define ADD_SCRIPT_PROPERTY(name, value)                                         \
    do {                                                                         \
        auto p = makeStringProperty(name, to_qstring(value.view()), grp_script); \
        p->on_set = [this](const QVariant& v) {                                  \
            value = nw::Resref{v.toString().toStdString()};                      \
        };                                                                       \
    } while (0)

    ADD_SCRIPT_PROPERTY("On Clicked", obj_->scripts.on_click);
    ADD_SCRIPT_PROPERTY("On Closed", obj_->scripts.on_closed);
    ADD_SCRIPT_PROPERTY("On Damaged", obj_->scripts.on_damaged);
    ADD_SCRIPT_PROPERTY("On Death", obj_->scripts.on_death);
    ADD_SCRIPT_PROPERTY("On Trap Disarmed", obj_->scripts.on_disarm);
    ADD_SCRIPT_PROPERTY("On Heartbeat", obj_->scripts.on_heartbeat);
    ADD_SCRIPT_PROPERTY("On Locked", obj_->scripts.on_lock);
    ADD_SCRIPT_PROPERTY("On Melee Attacked", obj_->scripts.on_melee_attacked);
    ADD_SCRIPT_PROPERTY("On Open", obj_->scripts.on_open);
    ADD_SCRIPT_PROPERTY("On Open Failure", obj_->scripts.on_open_failure);
    ADD_SCRIPT_PROPERTY("On Spell Cast At", obj_->scripts.on_spell_cast_at);
    ADD_SCRIPT_PROPERTY("On Trap Triggered", obj_->scripts.on_trap_triggered);
    ADD_SCRIPT_PROPERTY("On Unlock", obj_->scripts.on_unlock);
    ADD_SCRIPT_PROPERTY("On User Defined", obj_->scripts.on_user_defined);

#undef ADD_SCRIPT_PROPERTY

    addProperty(grp_script);
}

void DoorProperties::transitionLoad()
{
    Property* grp_trans = makeGroup("Transition");
    auto linked_to = makeStringProperty("Destination Tag", to_qstring(obj_->linked_to), grp_trans);
    linked_to->on_set = [this](const QVariant& value) {
        obj_->linked_to = value.toString().toStdString();
    };
    linked_to->setReadOnly(obj_->linked_to_flags == 0);

    QStringList types;
    types << "None"
          << "Door"
          << "Waypoint";

    auto list = new QStringListModel(types, this);
    auto linked_flags = makeEnumProperty("Destination Type", obj_->linked_to_flags, list, grp_trans);
    linked_flags->on_set = [this, linked_to](const QVariant& value) {
        obj_->linked_to_flags = static_cast<uint8_t>(value.toInt());
        linked_to->setReadOnly(obj_->linked_to_flags == 0);
        model()->updateReadOnly(linked_to);
    };

    addProperty(grp_trans);
}

void DoorProperties::trapsLoad()
{
    Property* grp_trap = makeGroup("Trap");

    trap_is_trapped_ = makeBoolProperty("Trapped", obj_->trap.is_trapped, grp_trap);
    trap_is_trapped_->on_set = [this](const QVariant& value) {
        obj_->trap.is_trapped = value.toBool();
        trapsUpdate();
    };

    trap_type_ = makeEnumProperty("Type", *obj_->trap.type, toolset().trap_model.get(), grp_trap);
    trap_type_->on_set = [this](const QVariant& value) {
        obj_->trap.type = nw::TrapType::make(value.toInt());
    };

    trap_detectable_ = makeBoolProperty("Detectable", obj_->trap.detectable, grp_trap);
    trap_detectable_->on_set = [this](const QVariant& value) {
        obj_->trap.detectable = value.toBool();
        trapsUpdate();
    };

    trap_detect_dc_ = makeIntegerProperty("Detection DC", obj_->trap.detect_dc, grp_trap);
    trap_detect_dc_->int_config.min = 0;
    trap_detect_dc_->int_config.max = 255;
    trap_detect_dc_->on_set = [this](const QVariant& value) {
        obj_->trap.detect_dc = static_cast<uint8_t>(value.toInt());
        trapsUpdate();
    };

    trap_disarmable_ = makeBoolProperty("Disarmable", obj_->trap.disarmable, grp_trap);
    trap_disarmable_->on_set = [this](const QVariant& value) {
        obj_->trap.disarmable = value.toBool();
        trapsUpdate();
    };

    trap_disarm_dc_ = makeIntegerProperty("Disarm DC", obj_->trap.disarm_dc, grp_trap);
    trap_disarm_dc_->int_config.min = 0;
    trap_disarm_dc_->int_config.max = 255;
    trap_detect_dc_->on_set = [this](const QVariant& value) {
        obj_->trap.disarm_dc = static_cast<uint8_t>(value.toInt());
        trapsUpdate();
    };

    trap_one_shot_ = makeBoolProperty("One Shot", obj_->trap.one_shot, grp_trap);
    trap_one_shot_->on_set = [this](const QVariant& value) {
        obj_->trap.one_shot = value.toBool();
        trapsUpdate();
    };

    trapsUpdate();
    addProperty(grp_trap);
}

void DoorProperties::trapsUpdate()
{
    trap_type_->setEditable(obj_->trap.is_trapped);
    model()->updateReadOnly(trap_type_);
    trap_is_trapped_->setEditable(true);
    model()->updateReadOnly(trap_is_trapped_);
    trap_detectable_->setEditable(obj_->trap.is_trapped);
    model()->updateReadOnly(trap_detectable_);
    trap_detect_dc_->setEditable(obj_->trap.detectable);
    model()->updateReadOnly(trap_detect_dc_);
    trap_disarmable_->setEditable(obj_->trap.is_trapped);
    model()->updateReadOnly(trap_disarmable_);
    trap_disarm_dc_->setEditable(obj_->trap.disarmable);
    model()->updateReadOnly(trap_disarm_dc_);
    trap_one_shot_->setEditable(obj_->trap.is_trapped);
    model()->updateReadOnly(trap_one_shot_);
}
