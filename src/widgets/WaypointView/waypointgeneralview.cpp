#include "waypointgeneralview.h"
#include "ui_waypointgeneralview.h"

#include "../../services/toolsetservice.h"
#include "../util/itemmodels.h"
#include "../util/strings.h"

#include "nw/objects/Waypoint.hpp"

#include <QStandardItemModel>

WaypointGeneralView::WaypointGeneralView(nw::Waypoint* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::WaypointGeneralView)
    , obj_{obj}
{
    ui->setupUi(this);

    ui->name->setLocString(obj_->common.name);
    ui->tag->setText(to_qstring(obj_->tag().view()));
    ui->resref->setText(to_qstring(obj_->common.resref.view()));

    ui->properties->setUndoStack(undoStack());
    auto index = findStandardItemIndex(toolset().waypoint_model.get(), obj_->appearance);
    auto type = ui->properties->makeEnumProperty("Appearance Type", index, toolset().waypoint_model.get());
    ui->properties->addProperty(type);

    auto mn_grp = ui->properties->makeGroup("Map Note");
    ui->properties->makeBoolProperty("Has Map Note", obj_->has_map_note, mn_grp);
    ui->properties->makeBoolProperty("Enabled", obj_->map_note_enabled, mn_grp);
}

WaypointGeneralView::~WaypointGeneralView()
{
    delete ui;
}
