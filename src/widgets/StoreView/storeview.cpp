#include "storeview.h"
#include "ui_storeview.h"

#include "../VariableTableView/variabletableview.h"
#include "../util/strings.h"
#include "storegeneralview.h"
#include "storeinventoryview.h"

#include "nw/kernel/Objects.hpp"
#include "nw/objects/Store.hpp"

#include <QTextEdit>

// == StoreView ===============================================================
// ============================================================================

StoreView::StoreView(nw::Resource res, QWidget* parent)
    : StoreView(nw::kernel::objects().load<nw::Store>(res.resref), parent)
{
    owned_ = true;
}

StoreView::StoreView(nw::Store* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::StoreView)
    , obj_{obj}
{
    ui->setupUi(this);

    auto gen = new StoreGeneralView(obj_, this);
    gen->setEnabled(!readOnly());
    ui->tabWidget->addTab(gen, "General");
    addTab(gen);

    auto inv = new StoreInventoryView(obj_, this);
    inv->setEnabled(!readOnly());
    ui->tabWidget->addTab(inv, "Inventory");
    addTab(inv);

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj_->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
    addTab(variables);

    auto comments = new QTextEdit(this);
    comments->setText(to_qstring(obj_->common.comment));
    ui->tabWidget->addTab(comments, "Comments");
}

StoreView::~StoreView()
{
    delete ui;
}
