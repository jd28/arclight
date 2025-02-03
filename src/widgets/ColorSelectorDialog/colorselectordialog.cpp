#include "colorselectordialog.h"

#include "colorselectorview.h"

#include <QShortcut>
#include <QUndoStack>
#include <QVBoxLayout>

ColorSelectionDialog::ColorSelectionDialog(nw::Item* obj, bool has_parts, QWidget* parent)
    : QDialog(parent)
    , undo_{new QUndoStack(this)}

{
    setWindowTitle("Select Item Colors");

    QVBoxLayout* layout = new QVBoxLayout(this);
    selector_ = new ColorSelectorView(obj, has_parts, undo_, this);
    layout->addWidget(selector_);

    QShortcut* undoShortcut = new QShortcut(QKeySequence::Undo, this);
    QShortcut* redoShortcut = new QShortcut(QKeySequence::Redo, this);

    connect(undoShortcut, &QShortcut::activated, undo_, &QUndoStack::undo);
    connect(redoShortcut, &QShortcut::activated, undo_, &QUndoStack::redo);
}

ColorSelectorView* ColorSelectionDialog::selector() const
{
    return selector_;
}
