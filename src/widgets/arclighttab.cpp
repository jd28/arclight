#include "arclighttab.h"

#include "ArclightView.h"

#include <QShortcut>
#include <QUndoStack>

ArclightTab::ArclightTab(ArclightView* parent)
    : QWidget{parent}
    , undo_stack_{new QUndoStack(this)}
{
    // Chain this up to main window.
    connect(this, &ArclightTab::activateUndoStack, parent, &ArclightView::activateUndoStack);

    QShortcut* us = new QShortcut(QKeySequence::Undo, this);
    QShortcut* rs = new QShortcut(QKeySequence::Redo, this);
    connect(us, &QShortcut::activated, undo_stack_, &QUndoStack::undo);
    connect(rs, &QShortcut::activated, undo_stack_, &QUndoStack::redo);
}

QUndoStack* ArclightTab::undoStack() const noexcept
{
    return undo_stack_;
}
