#ifndef SPINBOXDELEGATE_H
#define SPINBOXDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class QSpinBox;

class SpinBoxDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    using ConfigFunc = std::function<void(QSpinBox*, const QModelIndex&)>;

    explicit SpinBoxDelegate(ConfigFunc config, QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;

private:
    ConfigFunc config_;
};
#endif // SPINBOXDELEGATE_H
