#include "triggerview.h"
#include "ui_triggerview.h"

// == TriggerView =============================================================
// ============================================================================

TriggerView::TriggerView(QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::TriggerView)
{
    ui->setupUi(this);
}

TriggerView::~TriggerView()
{
    delete ui;
}
