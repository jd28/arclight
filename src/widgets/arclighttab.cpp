#include "arclighttab.h"

#include "ArclightView.h"

#include <QShortcut>
#include <QUndoStack>

ArclightTab::ArclightTab(ArclightView* parent)
    : QWidget{parent}
    , undo_{new QUndoStack(this)}
{
    connect(this, &ArclightTab::activateUndoStack, parent, &ArclightView::activateUndoStack);

    QShortcut* us = new QShortcut(QKeySequence::Undo, this);
    QShortcut* rs = new QShortcut(QKeySequence::Redo, this);
    connect(us, &QShortcut::activated, undo_, &QUndoStack::undo);
    connect(rs, &QShortcut::activated, undo_, &QUndoStack::redo);

    connect(undo_, &QUndoStack::cleanChanged, this, [this](bool clean) {
        modified_ = !clean;
        emit modificationChanged(!clean);
    });
    undo_->setClean();
}

ArclightTab::~ArclightTab()
{
    disconnect(undo_, &QUndoStack::cleanChanged, this, nullptr);
}

bool ArclightTab::modified() const noexcept
{
    return modified_;
}

QUndoStack* ArclightTab::undoStack() const noexcept
{
    return undo_;
}
