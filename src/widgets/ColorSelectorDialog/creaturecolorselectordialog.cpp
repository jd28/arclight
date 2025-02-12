#include "creaturecolorselectordialog.h"

#include "creaturecolorselectorview.h"

#include <QShortcut>
#include <QUndoStack>
#include <QVBoxLayout>

CreatureColorSelectionDialog::CreatureColorSelectionDialog(nw::Creature* obj, QUndoStack* undo, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Select Character Colors");

    QVBoxLayout* layout = new QVBoxLayout(this);
    selector_ = new CreatureColorSelectorView(obj, undo, this);
    layout->addWidget(selector_);

    QShortcut* undoShortcut = new QShortcut(QKeySequence::Undo, this);
    QShortcut* redoShortcut = new QShortcut(QKeySequence::Redo, this);

    connect(undoShortcut, &QShortcut::activated, undo, &QUndoStack::undo);
    connect(redoShortcut, &QShortcut::activated, undo, &QUndoStack::redo);
}
