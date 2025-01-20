#include "checkboxdelegate.h"

#include "nw/log.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QMouseEvent>

CheckBoxDelegate::CheckBoxDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

bool CheckBoxDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(option);

    if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonDblClick)) {
        auto* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() != Qt::LeftButton || !checkboxRect(option).contains(mouse_event->pos())) {
            return false;
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            return false;
        }
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
            return false;
        }
    } else {
        return false;
    }

    bool checked = index.data(Qt::DisplayRole).toBool();
    return model->setData(index, !checked, Qt::EditRole);
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
