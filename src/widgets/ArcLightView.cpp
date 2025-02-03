#include "ArclightView.h"

#include "../arclight/mainwindow.h"

#include <QUndoStack>

ArclightView::ArclightView(QWidget* parent)
    : QWidget(parent)
    , undo_stack_{new QUndoStack(this)}
{
    setFocusPolicy(Qt::StrongFocus);
}

bool ArclightView::readOnly() const noexcept
{
    return read_only_;
}

QUndoStack* ArclightView::undoStack() const noexcept
{
    return undo_stack_;
}

void ArclightView::focusInEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
    emit activateUndoStack(undo_stack_);
}
