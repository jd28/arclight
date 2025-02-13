#include "ArclightView.h"

#include "arclighttab.h"

#include "nw/log.hpp"

#include <QUndoStack>

ArclightView::ArclightView(QWidget* parent)
    : QWidget(parent)
{
}

void ArclightView::addTab(ArclightTab* tab)
{
    connect(tab, &ArclightTab::modificationChanged, this, &ArclightView::onModificationChanged);
    tabs_.append(tab);
}

bool ArclightView::modified() const noexcept
{
    return modified_;
}

bool ArclightView::readOnly() const noexcept
{
    return read_only_;
}

void ArclightView::onModificationChanged(bool modified)
{
    Q_UNUSED(modified);
    if (read_only_ || tabs_.empty()) { return; }

    bool mod = tabs_[0]->modified();
    for (qsizetype i = 1; i < tabs_.size(); ++i) {
        mod = mod || tabs_[i]->modified();
    }
    modified_ = mod;
    emit modificationChanged(modified_);
}
