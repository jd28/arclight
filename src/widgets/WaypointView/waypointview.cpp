#include "waypointview.h"
#include "ui_waypointview.h"

#include "../VariableTableView/variabletableview.h"
#include "waypointgeneralview.h"

#include "nw/kernel/Objects.hpp"

WaypointView::WaypointView(nw::Resource res, QWidget* parent)
    : WaypointView(nw::kernel::objects().load<nw::Waypoint>(res.resref), parent)
{
    owned_ = true;
}

WaypointView::WaypointView(nw::Waypoint* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::WaypointView)
    , obj_{obj}
{
    ui->setupUi(this);

    auto general = new WaypointGeneralView(obj_, this);
    general->setEnabled(!readOnly());
    ui->tabWidget->addTab(general, tr("General"));
    addTab(general);
    connect(general, &WaypointGeneralView::modificationChanged, this, &WaypointView::onModificationChanged);

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj_->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
    connect(variables, &VariableTableView::modificationChanged, this, &WaypointView::onModificationChanged);
}

WaypointView::~WaypointView()
{
    delete ui;
    if (owned_) {
        nw::kernel::objects().destroy(obj_->handle());
    }
}
