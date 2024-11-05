#ifndef CREATUREINVENTORYVIEW_H
#define CREATUREINVENTORYVIEW_H

#include <QAbstractItemModel>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QWidget>

namespace nw {
struct Creature;
enum struct EquipSlot;
struct Item;
}

class CreatureEquipView;

// == InventoryTable ==========================================================
// ============================================================================

class InventoryTable : public QTableView {
    Q_OBJECT

public:
    explicit InventoryTable(QWidget* parent = nullptr);

signals:
    void unequipItem(nw::Item* item, nw::EquipSlot slot);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};

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

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void addItem(nw::Item* item);
    nw::Creature* creature() const noexcept;
    void removeItem(nw::Item* item);

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

    void connectSlots(CreatureEquipView* equips);
    InventoryModel* model() const noexcept { return model_; }
    void setCreature(nw::Creature* creature);

public slots:
    void removeItemFromInventory(nw::Item* item);
    void addItemToInventory(nw::Item* item);
    void onRemove(bool checked = false);

private:
    Ui::CreatureInventoryView* ui;
    nw::Creature* creature_ = nullptr;
    InventoryModel* model_ = nullptr;
};

#endif // CREATUREINVENTORYVIEW_H
