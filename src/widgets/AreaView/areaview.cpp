#include "areaview.h"
#include "ui_areaview.h"

#include "nw/kernel/Objects.hpp"

AreaView::AreaView(nw::Resource area, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::AreaView)
    , obj_{nw::kernel::objects().make_area(area.resref)}
{
    if (!obj_) { return; }
    obj_->instantiate();

    ui->setupUi(this);
    loadModel();
}

AreaView::~AreaView()
{
    delete ui;
    if (obj_) {
        nw::kernel::objects().destroy(obj_->handle());
    }
}

void AreaView::loadModel()
{
    ui->openGLWidget->setFocus(Qt::ActiveWindowFocusReason);
    area_model_ = std::make_unique<BasicTileArea>(obj_);
    area_model_->load_tile_models();
    ui->openGLWidget->setNode(area_model_.get());
}
