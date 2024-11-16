#ifndef COLORSELECTORDIALOG_H
#define COLORSELECTORDIALOG_H

#include "nw/rules/items.hpp"

class ColorSelectorView;

#include <QDialog>

class ColorSelectionDialog : public QDialog {
    Q_OBJECT

public:
    ColorSelectionDialog(QWidget* parent = nullptr);

    void setIndex(nw::ItemColors::type index);

private:
    ColorSelectorView* selector_;
    nw::ItemColors::type index_;
};

#endif // COLORSELECTORDIALOG_H
