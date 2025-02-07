#include "itemsimplemodelselectordialog.h"
#include "ui_itemsimplemodelselectordialog.h"

ItemSimpleModelSelectorDialog::ItemSimpleModelSelectorDialog(nw::BaseItem type, int current, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ItemSimpleModelSelectorDialog)
{
    ui->setupUi(this);
    selector_ = new ItemSimpleModelSelector(type, current, this);
    ui->layout->addWidget(selector_);
}

ItemSimpleModelSelectorDialog::~ItemSimpleModelSelectorDialog()
{
    delete ui;
}

ItemSimpleModelSelector* ItemSimpleModelSelectorDialog::selector() const
{
    return selector_;
}
