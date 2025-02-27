#include "placeableproperties.h"

#include "../../services/toolset/toolsetservice.h"
#include "../util/itemmodels.h"
#include "../util/strings.h"

#include "nw/kernel/Rules.hpp"
#include "nw/objects/Placeable.hpp"

#include <QCompleter>
#include <QStandardItemModel>

#include <limits>

static QRegularExpression resref_regex("^[a-z_]{1,16}$");

PlaceableProperties::PlaceableProperties(QWidget* parent)
    : PropertyBrowser{parent}
{
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
}

// == Private Methods =========================================================
// ============================================================================

void PlaceableProperties::basicsLoad()
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

    auto has_inventory = makeBoolProperty("Has Inventory", obj_->has_inventory, grp_basic);
    has_inventory->on_set = [this](const QVariant& value) {
        obj_->has_inventory = value.toBool();
        emit hasInventoryChanged(obj_->has_inventory);
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

    auto static_ = makeBoolProperty("Static", obj_->static_, grp_basic);
    static_->on_set = [this](const QVariant& value) {
        obj_->static_ = value.toBool();
        locksUpdate();
        trapsUpdate();
    };

    auto useable = makeBoolProperty("Useable", obj_->useable);
    useable->on_set = [this](const QVariant& value) {
        obj_->useable = value.toInt();
        locksUpdate();
        trapsUpdate();
    };

    addProperty(grp_basic);
}

void PlaceableProperties::conversationLoad()
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

void PlaceableProperties::locksLoad()
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

void PlaceableProperties::locksUpdate()
{
    lock_locked_prop_->setEditable(obj_->useable);
    model()->updateReadOnly(lock_locked_prop_);
    lock_lockable_prop_->setEditable(obj_->useable);
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

void PlaceableProperties::savesLoad()
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

void PlaceableProperties::scriptsLoad()
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
    ADD_SCRIPT_PROPERTY("On Inventory Disturbed", obj_->scripts.on_inventory_disturbed);
    ADD_SCRIPT_PROPERTY("On Locked", obj_->scripts.on_lock);
    ADD_SCRIPT_PROPERTY("On Melee Attacked", obj_->scripts.on_melee_attacked);
    ADD_SCRIPT_PROPERTY("On Open", obj_->scripts.on_open);
    ADD_SCRIPT_PROPERTY("On Spell Cast At", obj_->scripts.on_spell_cast_at);
    ADD_SCRIPT_PROPERTY("On Trap Triggered", obj_->scripts.on_trap_triggered);
    ADD_SCRIPT_PROPERTY("On Unlock", obj_->scripts.on_unlock);
    ADD_SCRIPT_PROPERTY("On Used", obj_->scripts.on_used);
    ADD_SCRIPT_PROPERTY("On User Defined", obj_->scripts.on_user_defined);

    addProperty(grp_script);
}

void PlaceableProperties::trapsLoad()
{
    Property* grp_trap = makeGroup("Trap");

    trap_is_trapped_ = makeBoolProperty("Trapped", obj_->trap.is_trapped, grp_trap);
    trap_is_trapped_->on_set = [this](const QVariant& value) {
        obj_->trap.is_trapped = value.toBool();
        trapsUpdate();
    };

    auto index = mapSourceRowToProxyRow(toolset().trap_model.get(), toolset().trap_filter.get(), *obj_->trap.type);
    trap_type_ = makeEnumProperty("Type", index, toolset().trap_filter.get(), grp_trap);
    trap_type_->on_set = [this](const QVariant& value) {
        int index = mapProxyRowToSourceRow(toolset().trap_filter.get(), value.toInt());
        obj_->trap.type = nw::TrapType::make(index);
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

void PlaceableProperties::trapsUpdate()
{
    trap_type_->setEditable(obj_->trap.is_trapped);
    model()->updateReadOnly(trap_type_);
    trap_is_trapped_->setEditable(obj_->useable);
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
