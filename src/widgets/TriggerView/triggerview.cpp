#include "triggerview.h"
#include "ui_triggerview.h"

#include "../VariableTableView/variabletableview.h"
#include "triggergeneralview.h"

// == TriggerView =============================================================
// ============================================================================

#include "nw/kernel/Objects.hpp"

TriggerView::TriggerView(nw::Resource res, QWidget* parent)
    : TriggerView(nw::kernel::objects().load<nw::Trigger>(res.resref), parent)
{
    owned_ = true;
}

TriggerView::TriggerView(nw::Trigger* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::TriggerView)
    , obj_{obj}
{
    ui->setupUi(this);

    auto general = new TriggerGeneralView(obj_, this);
    ui->tabWidget->addTab(general, "General");
    addTab(general);

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj_->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
}

TriggerView::~TriggerView()
{
    delete ui;
    if (owned_ && obj_) {
        nw::kernel::objects().destroy(obj_->handle());
    }
}
