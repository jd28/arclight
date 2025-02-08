#include "ArclightView.h"

#include "../arclight/mainwindow.h"

#include <QUndoStack>

ArclightView::ArclightView(QWidget* parent)
    : QWidget(parent)
{
}

bool ArclightView::isModified() const noexcept
{
    return modified_;
}

bool ArclightView::readOnly() const noexcept
{
    return read_only_;
}

void ArclightView::setModified(bool value)
{
    if (read_only_) { return; }
    bool already_modified = modified_;
    modified_ = value;
    if (!already_modified) {
        emit modified();
    }
}
