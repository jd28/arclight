#include "placeableview.h"
#include "ui_placeableview.h"

#include "../InventoryView/inventoryview.h"
#include "../VariableTableView/variabletableview.h"
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
    connect(general, &PlaceableGeneralView::modificationChanged, this, &PlaceableView::onModificationChanged);

    inventory_ = new InventoryView(this);
    inventory_->setEnabled(!readOnly() && obj_->has_inventory);
    inventory_->setObject(obj_, &obj_->inventory);
    inventory_->setDragEnabled(false);
    ui->tabWidget->addTab(inventory_, tr("Inventory"));

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj_->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
    connect(variables, &VariableTableView::modificationChanged, this, &PlaceableView::onModificationChanged);

    auto description = new StrrefTextEdit(this);
    description->setEnabled(!readOnly());
    description->setLocstring(obj->description);
    ui->tabWidget->addTab(description, tr("Description"));

    auto comments = new QTextEdit(this);
    comments->setEnabled(!readOnly());
    comments->setProperty("last_text", to_qstring(obj_->common.comment));
    comments->setText(to_qstring(obj_->common.comment));
    ui->tabWidget->addTab(comments, tr("Comments"));

    loadModel();
    connect(general, &PlaceableGeneralView::appearanceChanged, this, &PlaceableView::loadModel);
}

PlaceableView::~PlaceableView()
{
    delete ui;
}

void PlaceableView::loadModel()
{
    LOG_F(INFO, "loadModel called for appearance: {}", obj_->appearance);
    auto plc = nw::kernel::rules().placeables.get(nw::PlaceableType::make(obj_->appearance));
    if (!plc) {
        LOG_F(ERROR, "No placeable data for appearance: {}", obj_->appearance);
        return;
    }
    if (plc->model.empty()) {
        LOG_F(WARNING, "Empty model string for appearance: {}", obj_->appearance);
        return;
    }

    LOG_F(INFO, "Loading model: {}", plc->model.view());
    auto model = load_model(plc->model.view());
    if (!model) {
        LOG_F(ERROR, "Failed to load model: {}", plc->model.view());
        return;
    }
    LOG_F(INFO, "Model loaded: {}, nodes={}", plc->model.view(), model->nodes_.size());
    ui->openGLWidget->setModel(std::move(model));
    LOG_F(INFO, "Model set on RenderWidget");
}

void PlaceableView::onHasInvetoryChanged(bool value)
{
    inventory_->setEnabled(value);
}
