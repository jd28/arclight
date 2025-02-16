#include "encounterview.h"
#include "ui_encounterview.h"

#include "../VariableTableView/variabletableview.h"
#include "../util/strings.h"
#include "encounterpropsview.h"

#include "nw/kernel/Objects.hpp"
#include "nw/objects/Encounter.hpp"

#include <QTextEdit>

EncounterView::EncounterView(nw::Resource res, QWidget* parent)
    : EncounterView(nw::kernel::objects().load<nw::Encounter>(res.resref), parent)
{
    owned_ = true;
}

EncounterView::EncounterView(nw::Encounter* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::EncounterView)
    , obj_{obj}
{
    if (!obj_) { return; }
    ui->setupUi(this);

    auto general = new EncounterPropsView(obj_, this);
    general->setEnabled(!readOnly());
    ui->tabWidget->addTab(general, "General");
    addTab(general);

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj_->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
    addTab(variables);

    auto comments = new QTextEdit(this);
    comments->setText(to_qstring(obj_->common.comment));
    ui->tabWidget->addTab(comments, "Comments");
}

EncounterView::~EncounterView()
{
    delete ui;
}
