#ifndef ITEMSIMPLEMODELSELECTORDIALOG_H
#define ITEMSIMPLEMODELSELECTORDIALOG_H

#include "itemsimplemodelselector.h"

#include <QDialog>

namespace Ui {
class ItemSimpleModelSelectorDialog;
}

class ItemSimpleModelSelectorDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItemSimpleModelSelectorDialog(nw::BaseItem type, int current, QWidget* parent = nullptr);
    ~ItemSimpleModelSelectorDialog();

    ItemSimpleModelSelector* selector() const;

private:
    Ui::ItemSimpleModelSelectorDialog* ui;
    ItemSimpleModelSelector* selector_;
};

#endif // ITEMSIMPLEMODELSELECTORDIALOG_H
