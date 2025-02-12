#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QWidget>

class CheckBoxDelegate : public QStyledItemDelegate {
public:
    CheckBoxDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    QRect checkboxRect(const QStyleOptionViewItem& viewItemStyleOptions) const;
};

#endif // CHECKBOXDELEGATE_H
