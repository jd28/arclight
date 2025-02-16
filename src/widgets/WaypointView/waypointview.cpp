#include "waypointview.h"
#include "ui_waypointview.h"

WaypointView::WaypointView(QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::WaypointView)
{
    ui->setupUi(this);
}

WaypointView::~WaypointView()
{
    delete ui;
}
