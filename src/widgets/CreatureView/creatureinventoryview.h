#ifndef CREATUREINVENTORYVIEW_H
#define CREATUREINVENTORYVIEW_H

#include <QAbstractItemModel>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QWidget>

namespace nw {
struct Creature;
struct Item;
}

// == InventoryItemDelegate ===================================================
// ============================================================================

constexpr int StackableRole = Qt::UserRole + 1;

class InventoryItemDelegate : public QStyledItemDelegate {
public:
    InventoryItemDelegate(QObject* parent = nullptr);

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

// == InventoryModel ==========================================================
// ============================================================================

class InventoryModel : public QAbstractTableModel {
    Q_OBJECT

public:
    InventoryModel(nw::Creature* creature, QObject* parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void addItem(nw::Item* item);

private:
    nw::Creature* creature_;
};

namespace Ui {
class CreatureInventoryView;
}

class CreatureInventoryView : public QWidget {
    Q_OBJECT

public:
    explicit CreatureInventoryView(QWidget* parent = nullptr);
    ~CreatureInventoryView();

    void setCreature(nw::Creature* creature);

private:
    Ui::CreatureInventoryView* ui;
    nw::Creature* creature_ = nullptr;
    InventoryModel* model_ = nullptr;
};

#endif // CREATUREINVENTORYVIEW_H
