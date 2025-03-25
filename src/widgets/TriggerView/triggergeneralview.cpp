#include "triggergeneralview.h"
#include "ui_triggergeneralview.h"

#include "../../services/toolset/toolsetservice.h"
#include "../util/itemmodels.h"
#include "../util/strings.h"

#include "nw/objects/Trigger.hpp"

#include <QStringListModel>

// == TriggerGeneralView ======================================================
// ============================================================================

TriggerGeneralView::TriggerGeneralView(nw::Trigger* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::TriggerGeneralView)
    , obj_{obj}
{
    ui->setupUi(this);

    ui->name->setLocString(obj_->common.name);
    ui->tag->setText(to_qstring(obj_->tag().view()));
    ui->resref->setText(to_qstring(obj_->common.resref.view()));

    ui->properties->setUndoStack(undoStack());

    QStringList types;
    types << "General" << "Transition" << "Trap";
    auto type_model = new QStringListModel(types, this);
    auto types_prop = ui->properties->makeEnumProperty("Type", obj_->type, type_model);
    types_prop->on_set = [this](const QVariant& value) {
        obj_->type = value.toInt();
        trapsUpdate();
    };
    ui->properties->addProperty(types_prop);

    auto highlite_height = ui->properties->makeDoubleProperty("Highlite Height", obj_->highlight_height);
    highlite_height->double_config.min = 0.1;
    highlite_height->double_config.step = 0.1;
    highlite_height->on_set = [this](const QVariant& value) {
        obj_->highlight_height = static_cast<float>(value.toDouble());
    };
    ui->properties->addProperty(highlite_height);

    transitionLoad();
    scriptsLoad();
    trapsLoad();
}

TriggerGeneralView::~TriggerGeneralView()
{
    delete ui;
}

void TriggerGeneralView::scriptsLoad()
{
#define ADD_SCRIPT(name, resref, grp)                                                                    \
    do {                                                                                                 \
        auto p = ui->properties->makeStringProperty(name, to_qstring(obj_->scripts.resref.view()), grp); \
        p->on_set = [this](const QVariant& value) {                                                      \
            obj_->scripts.resref = nw::Resref{value.toString().toStdString()};                           \
        };                                                                                               \
    } while (0)

    auto grp_scripts = ui->properties->makeGroup("Scripts");
    ADD_SCRIPT("On Click", on_click, grp_scripts);
    ADD_SCRIPT("On Enter", on_enter, grp_scripts);
    ADD_SCRIPT("On Exit", on_exit, grp_scripts);
    ADD_SCRIPT("On Heartbeat", on_heartbeat, grp_scripts);
    ADD_SCRIPT("On Trap Disarmed", on_disarm, grp_scripts);
    ADD_SCRIPT("On Trap Triggered", on_trap_triggered, grp_scripts);
    ADD_SCRIPT("On User Defined", on_user_defined, grp_scripts);
    ui->properties->addProperty(grp_scripts);
#undef ADD_SCRIPT
}

void TriggerGeneralView::transitionLoad()
{
    Property* grp_trans = ui->properties->makeGroup("Area Transition");
    auto linked_to = ui->properties->makeStringProperty("Destination Tag", to_qstring(obj_->linked_to), grp_trans);
    linked_to->on_set = [this](const QVariant& value) {
        obj_->linked_to = value.toString().toStdString();
    };
    linked_to->setReadOnly(obj_->type != 1);

    QStringList types;
    types << "None"
          << "Door"
          << "Waypoint";

    auto list = new QStringListModel(types, this);
    auto linked_flags = ui->properties->makeEnumProperty("Destination Type", obj_->linked_to_flags, list, grp_trans);
    linked_flags->setReadOnly(obj_->type != 1);
    linked_flags->on_set = [this](const QVariant& value) {
        obj_->linked_to_flags = static_cast<uint8_t>(value.toInt());
    };

    ui->properties->addProperty(grp_trans);
}

void TriggerGeneralView::trapsLoad()
{
    Property* grp_trap = ui->properties->makeGroup("Trap");

    ui->properties->makeBoolProperty("Trapped", obj_->trap.is_trapped, grp_trap);

    auto index = mapSourceRowToProxyRow(toolset().trap_model.get(), toolset().trap_filter.get(), *obj_->trap.type);
    trap_type_ = ui->properties->makeEnumProperty("Type", index, toolset().trap_filter.get(), grp_trap);
    trap_type_->on_set = [this](const QVariant& value) {
        int index = mapProxyRowToSourceRow(toolset().trap_filter.get(), value.toInt());
        obj_->trap.type = nw::TrapType::make(index);
    };

    trap_detectable_ = ui->properties->makeBoolProperty("Detectable", obj_->trap.detectable, grp_trap);
    trap_detectable_->on_set = [this](const QVariant& value) {
        obj_->trap.detectable = value.toBool();
        trapsUpdate();
    };

    trap_detect_dc_ = ui->properties->makeIntegerProperty("Detection DC", obj_->trap.detect_dc, grp_trap);
    trap_detect_dc_->int_config.min = 0;
    trap_detect_dc_->int_config.max = 255;
    trap_detect_dc_->on_set = [this](const QVariant& value) {
        obj_->trap.detect_dc = static_cast<uint8_t>(value.toInt());
        trapsUpdate();
    };

    trap_disarmable_ = ui->properties->makeBoolProperty("Disarmable", obj_->trap.disarmable, grp_trap);
    trap_disarmable_->on_set = [this](const QVariant& value) {
        obj_->trap.disarmable = value.toBool();
        trapsUpdate();
    };

    trap_disarm_dc_ = ui->properties->makeIntegerProperty("Disarm DC", obj_->trap.disarm_dc, grp_trap);
    trap_disarm_dc_->int_config.min = 0;
    trap_disarm_dc_->int_config.max = 255;
    trap_detect_dc_->on_set = [this](const QVariant& value) {
        obj_->trap.disarm_dc = static_cast<uint8_t>(value.toInt());
        trapsUpdate();
    };

    trap_one_shot_ = ui->properties->makeBoolProperty("One Shot", obj_->trap.one_shot, grp_trap);
    trap_one_shot_->on_set = [this](const QVariant& value) {
        obj_->trap.one_shot = value.toBool();
        trapsUpdate();
    };

    trapsUpdate();
    ui->properties->addProperty(grp_trap);
}

void TriggerGeneralView::trapsUpdate()
{
    trap_type_->setEditable(obj_->type == 2);
    ui->properties->model()->updateReadOnly(trap_type_);
    trap_detectable_->setEditable(obj_->type == 2);
    ui->properties->model()->updateReadOnly(trap_detectable_);
    trap_detect_dc_->setEditable(obj_->type == 2);
    ui->properties->model()->updateReadOnly(trap_detect_dc_);
    trap_disarmable_->setEditable(obj_->type == 2);
    ui->properties->model()->updateReadOnly(trap_disarmable_);
    trap_disarm_dc_->setEditable(obj_->type == 2);
    ui->properties->model()->updateReadOnly(trap_disarm_dc_);
    trap_one_shot_->setEditable(obj_->type == 2);
    ui->properties->model()->updateReadOnly(trap_one_shot_);
}
