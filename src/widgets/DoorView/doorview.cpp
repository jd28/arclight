#include "doorview.h"
#include "ui_doorview.h"

#include "../strreftextedit.h"
#include "../util/strings.h"
#include "doorgeneralview.h"

#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Door.hpp"

#include <QScreen>
#include <QTextEdit>

DoorView::DoorView(nw::Door* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::DoorView)
    , obj_{obj}
{
    ui->setupUi(this);

    auto width = qApp->primaryScreen()->geometry().width();
    ui->splitter->setSizes(QList<int>() << width * 1 / 3 << width * 2 / 3);

    auto general = new DoorGeneralView(obj_, this);
    ui->tabWidget->addTab(general, "General");
    auto description = new StrrefTextEdit(this);
    description->setLocstring(obj->description);
    ui->tabWidget->addTab(description, "Description");
    auto comments = new QTextEdit(this);
    comments->setText(to_qstring(obj_->common.comment));
    ui->tabWidget->addTab(comments, "Comments");

    connect(ui->openGLWidget, &BasicModelView::initialized, this, &DoorView::loadModel);
    connect(general, &DoorGeneralView::appearanceChanged, this, &DoorView::loadModel);
}

DoorView::~DoorView()
{
    delete ui;
}

void DoorView::loadModel()
{
    ui->openGLWidget->makeCurrent();
    if (obj_->appearance == 0) {
        auto genericdoors = nw::kernel::twodas().get("genericdoors");
        if (genericdoors) {
            std::string model;
            if (genericdoors->get_to(obj_->generic_type, "ModelName", model)) {
                auto mdl = load_model(model, ui->openGLWidget->funcs());
                ui->openGLWidget->setModel(std::move(mdl));
            }
        } else {
            throw std::runtime_error("[door] failed to load genericdoors.2da");
        }
    } else {
        auto doortypes = nw::kernel::twodas().get("doortypes");
        if (doortypes) {
            std::string model;
            if (doortypes->get_to(obj_->appearance, "Model", model)) {
                auto mdl = load_model(model, ui->openGLWidget->funcs());
                ui->openGLWidget->setModel(std::move(mdl));
            }
        } else {
            throw std::runtime_error("[door] failed to load doortypes.2da");
        }
    }
}
