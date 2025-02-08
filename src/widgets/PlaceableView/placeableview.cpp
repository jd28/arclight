#include "placeableview.h"
#include "ui_placeableview.h"

#include "../strreftextedit.h"
#include "../util/strings.h"
#include "placeablegeneralview.h"

#include "nw/kernel/Objects.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/objects/Placeable.hpp"

#include <QScreen>
#include <QTextEdit>

PlaceableView::PlaceableView(nw::Resource res, QWidget* parent)
    : PlaceableView(nw::kernel::objects().load<nw::Placeable>(res.resref), parent)
{
    owned_ = true;
}

PlaceableView::PlaceableView(nw::Placeable* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::PlaceableView)
    , obj_{obj}
{
    if (!obj_) { return; }

    ui->setupUi(this);

    auto width = qApp->primaryScreen()->geometry().width();
    ui->splitter->setSizes(QList<int>() << width * 1 / 3 << width * 2 / 3);

    auto general = new PlaceableGeneralView(obj_, this);
    ui->tabWidget->addTab(general, "General");
    auto description = new StrrefTextEdit(this);
    description->setLocstring(obj->description);
    ui->tabWidget->addTab(description, "Description");
    auto comments = new QTextEdit(this);
    comments->setText(to_qstring(obj_->common.comment));
    ui->tabWidget->addTab(comments, "Comments");

    connect(ui->openGLWidget, &BasicModelView::initialized, this, &PlaceableView::loadModel);
    connect(general, &PlaceableGeneralView::appearanceChanged, this, &PlaceableView::loadModel);
}

PlaceableView::~PlaceableView()
{
    delete ui;
}

void PlaceableView::loadModel()
{
    ui->openGLWidget->makeCurrent();
    auto plc = nw::kernel::rules().placeables.get(nw::PlaceableType::make(obj_->appearance));
    if (plc && !plc->model.empty()) {
        ui->openGLWidget->setModel(load_model(plc->model.view(), ui->openGLWidget->funcs()));
    }
}
