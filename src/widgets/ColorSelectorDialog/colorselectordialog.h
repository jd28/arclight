#ifndef COLORSELECTORDIALOG_H
#define COLORSELECTORDIALOG_H

#include "nw/rules/items.hpp"

namespace nw {
struct Item;
}

class ColorSelectorView;
class QUndoStack;

#include <QDialog>

class ColorSelectionDialog : public QDialog {
    Q_OBJECT

public:
    ColorSelectionDialog(nw::Item* obj, bool has_parts, QWidget* parent = nullptr);
    ColorSelectorView* selector() const;

private:
    ColorSelectorView* selector_;
    nw::ItemColors::type index_;
    QUndoStack* undo_;
};

#endif // COLORSELECTORDIALOG_H
