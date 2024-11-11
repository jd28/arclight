#include "strreflineedit.h"

#include "nw/kernel/Strings.hpp"

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include "util/strings.h"

StrrefLineEdit::StrrefLineEdit(nw::LocString value, QWidget* parent)
    : StrrefLineEdit(parent)
{
    setLocString(std::move(value));
}

StrrefLineEdit::StrrefLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    auto action = addAction(ZFontIcon::icon(Fa6::FAMILY, Fa6::SOLID, Fa6::fa_ellipsis, Qt::gray), QLineEdit::TrailingPosition);
    action->setToolTip("Edit Localized String");

    connect(action, &QAction::triggered, this, &StrrefLineEdit::onActionTriggered);
}

void StrrefLineEdit::setLocString(nw::LocString value)
{
    value_ = std::move(value);
    setText(to_qstring(nw::kernel::strings().get(value_)));
}

void StrrefLineEdit::onActionTriggered()
{
    emit editClicked(value_);
}
