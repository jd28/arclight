#include "spinboxdelegate.h"

#include <QSpinBox>

SpinBoxDelegate::SpinBoxDelegate(ConfigFunc config, QObject* parent)
    : QStyledItemDelegate(parent)
    , config_(std::move(config))
{
}

QWidget* SpinBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    auto* spinBox = new QSpinBox(parent);
    if (config_) {
        config_(spinBox, index); // Apply custom configuration
    }
    return spinBox;
}

void SpinBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto value = index.model()->data(index, Qt::DisplayRole);
    if (auto* spinBox = qobject_cast<QSpinBox*>(editor)) {
        spinBox->setValue(value.toInt());
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void SpinBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
    const QModelIndex& index) const
{
    if (auto* spinBox = qobject_cast<QSpinBox*>(editor)) {
        spinBox->interpretText();
        model->setData(index, spinBox->value(), Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
