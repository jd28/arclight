#ifndef VENTORYVIEW_H
#define VENTORYVIEW_H

#include <QAbstractItemModel>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QWidget>

namespace nw {
struct ObjectBase;
struct Creature;
enum struct EquipSlot;
struct Inventory;
struct Item;
struct Placable;
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
    void mouseReleaseEvent(QMouseEvent* event) override;
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
    InventoryModel(nw::ObjectBase* obj, nw::Inventory* inventory, int store_tab, QObject* parent = nullptr);

    void addItem(nw::Item* item);
    nw::ObjectBase* object() const noexcept;
    void removeItem(nw::Item* item);
    int store_tab() const noexcept;

    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::DropActions supportedDropActions() const override;

private:
    nw::ObjectBase* obj_;
    nw::Inventory* inventory_;
    int store_tab_ = -1;
};

namespace Ui {
class InventoryView;
}

class InventoryView : public QWidget {
    Q_OBJECT

public:
    explicit InventoryView(QWidget* parent = nullptr);
    ~InventoryView();

    void connectSlots(CreatureEquipView* equips);
    InventoryModel* model() const noexcept { return model_; }
    void setDragEnabled(bool value);
    void setObject(nw::ObjectBase* obj, nw::Inventory* inventory, int store_tab = -1);

public slots:
    void removeItemFromInventory(nw::Item* item);
    void addItemToInventory(nw::Item* item);
    void onRemove(bool checked = false);

private:
    Ui::InventoryView* ui;
    nw::ObjectBase* obj_ = nullptr;
    InventoryModel* model_ = nullptr;
};

#endif // VENTORYVIEW_H
