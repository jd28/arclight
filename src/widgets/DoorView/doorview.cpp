#include "doorview.h"
#include "ui_doorview.h"

#include "../VariableTableView/variabletableview.h"
#include "../loadscreensview.h"
#include "../strreftextedit.h"
#include "../util/strings.h"
#include "doorgeneralview.h"

#include "nw/kernel/Objects.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Door.hpp"

#include <QScreen>
#include <QTextEdit>

DoorView::DoorView(nw::Resource res, QWidget* parent)
    : DoorView(nw::kernel::objects().load<nw::Door>(res.resref), parent)
{
    owned_ = true;
}

DoorView::DoorView(nw::Door* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::DoorView)
    , obj_{obj}
{
    if (!obj_) { return; }

    ui->setupUi(this);
    auto width = qApp->primaryScreen()->geometry().width();
    ui->splitter->setSizes(QList<int>() << width * 6 / 10 << width * 4 / 10);

    auto general = new DoorGeneralView(obj_, this);
    general->setEnabled(!readOnly());
    ui->tabWidget->addTab(general, "General");
    addTab(general);

    auto loadscreens = new LoadscreensView(obj_->loadscreen, this);
    loadscreens->setEnabled(!readOnly());
    ui->tabWidget->addTab(loadscreens, tr("Loadscreens"));
    addTab(loadscreens);
    connect(loadscreens, &LoadscreensView::valueChanged, this, [this](int value) {
        obj_->loadscreen = uint16_t(value);
    });

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj_->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
    addTab(variables);

    auto description = new StrrefTextEdit(this);
    description->setLocstring(obj->description);
    ui->tabWidget->addTab(description, "Description");

    auto comments = new QTextEdit(this);
    comments->setText(to_qstring(obj_->common.comment));
    ui->tabWidget->addTab(comments, "Comments");

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &DoorView::onTabChanged);
    connect(ui->openGLWidget, &BasicModelView::initialized, this, &DoorView::loadModel);
    connect(general, &DoorGeneralView::appearanceChanged, this, &DoorView::loadModel);
}

DoorView::~DoorView()
{
    delete ui;
    if (owned_) {
        nw::kernel::objects().destroy(obj_->handle());
    }
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

void DoorView::onTabChanged(int index)
{
    // ui->openGLWidget->setHidden(index == 1);
}
