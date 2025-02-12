#include "checkboxdelegate.h"

#include "nw/log.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QMouseEvent>

CheckBoxDelegate::CheckBoxDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void CheckBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
    QStyleOptionButton styleOptionButton;
    styleOptionButton.state |= QStyle::State_Enabled;
    if (checked) {
        styleOptionButton.state |= QStyle::State_On;
    } else {
        styleOptionButton.state |= QStyle::State_Off;
    }

    styleOptionButton.rect = checkboxRect(option);

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &styleOptionButton, painter);
}

QRect CheckBoxDelegate::checkboxRect(const QStyleOptionViewItem& viewItemStyleOptions) const
{
    QStyleOptionButton styleOptionButton;
    QRect rect = QApplication::style()->subElementRect(
        QStyle::SE_CheckBoxIndicator, &styleOptionButton);
    QPoint point(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - rect.width() / 2,
        viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - rect.height() / 2);
    return QRect(point, rect.size());
}
