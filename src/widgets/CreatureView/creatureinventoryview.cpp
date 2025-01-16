#include "creatureinventoryview.h"
#include "ui_creatureinventoryview.h"

#include "../util/objects.h"
#include "../util/strings.h"
#include "creatureequipview.h"

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include "nw/kernel/Objects.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/log.hpp"
#include "nw/objects/Creature.hpp"
#include "nw/objects/Item.hpp"

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

InventoryTable::InventoryTable(QWidget* parent)
    : QTableView(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

void InventoryTable::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QModelIndex index = indexAt(event->pos());
        auto idx = model()->index(index.row(), 0, index.parent());
        if (index.isValid()) {
            QMimeData* mimeData = new QMimeData();
            nw::Item* item = reinterpret_cast<nw::Item*>(idx.internalPointer());
            mimeData->setData("application/x-inventory-item", serialize_obj_handle(item->handle()));

            QPixmap img = model()->data(idx, Qt::DecorationRole).value<QPixmap>();
            QPoint hotspot = QPoint(img.width() / 2, img.height() / 2);

            QDrag* drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setPixmap(img);
            drag->setHotSpot(hotspot);
            drag->exec(Qt::MoveAction);
        }
    }
    QTableView::mousePressEvent(event);
}

void InventoryTable::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->source() == this) {
        event->ignore();
    } else if (event->mimeData()->hasFormat("application/x-equip-item")) {
        event->acceptProposedAction();
    }
}

void InventoryTable::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->source() == this) {
        event->ignore();
    } else if (event->mimeData()->hasFormat("application/x-equip-item")) {
        event->acceptProposedAction();
    }
}

void InventoryTable::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-equip-item")) {
        QByteArray itemData = event->mimeData()->data("application/x-equip-item");
        auto item_handle = deserialize_obj_handle(itemData);
        auto item = nw::kernel::objects().get<nw::Item>(item_handle);
        if (!item) { return; }

        auto m = static_cast<InventoryModel*>(model());
        int i = 0;
        for (const auto& it : m->creature()->equipment.equips) {
            if (it.is<nw::Item*>() && it.as<nw::Item*>() == item) { break; }
            ++i;
        }

        auto slot = static_cast<nw::EquipSlot>(1 << i);
        emit unequipItem(item, slot);

        m->addItem(item);
        resizeRowsToContents();
        event->acceptProposedAction();
    }
}

QModelIndex InventoryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    nw::Item* item = creature_->inventory.items[row].item.as<nw::Item*>();
    if (!item) { return QModelIndex(); }

    return createIndex(row, column, item);
}
// == InventoryItemDelegate ===================================================
// ============================================================================

InventoryItemDelegate::InventoryItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
    // Create a QModelIndex with the pointer as the internal data
}

void InventoryItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->displayAlignment = Qt::AlignCenter;
}

void InventoryItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.column() == 0) {
        QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();
        if (!pixmap.isNull()) {
            if (pixmap.height() > 128) {
                pixmap = pixmap.scaled(QSize(64, 128), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            int x = option.rect.x() + (option.rect.width() - pixmap.width()) / 2;
            int y = option.rect.y() + (option.rect.height() - pixmap.height()) / 2;
            painter->drawPixmap(x, y, pixmap);
            return;
        }
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QSize InventoryItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.column() == 0) {
        QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();
        int height = qMin(pixmap.height(), 128);

        QFontMetrics fontMetrics(option.font);
        int text = fontMetrics.height();

        int row = qMax(height, text) + 8;
        return QSize(option.rect.width(), row);
    }
    return QStyledItemDelegate::sizeHint(option, index);
}

// == InventoryModel ==========================================================
// ============================================================================

InventoryModel::InventoryModel(nw::Creature* creature, QObject* parent)
    : QAbstractTableModel(parent)
    , creature_{creature}
{
}

QVariant InventoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return "Icon";
        case 1:
            return "Name";
        case 2:
            return "Stack Size";
        default:
            return QVariant();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

int InventoryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return creature_ ? static_cast<int>(creature_->inventory.items.size()) : 0;
}

int InventoryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant InventoryModel::data(const QModelIndex& index, int role) const
{
    if (!creature_) { return QVariant(); }
    if (!index.isValid() || index.row() >= static_cast<int>(creature_->inventory.items.size())) {
        return QVariant();
    }

    if (!creature_->inventory.items[index.row()].item.is<nw::Item*>()) {
        return {};
    }

    const auto item = creature_->inventory.items[index.row()].item.as<nw::Item*>();
    if (!item) { return {}; }

    switch (index.column()) {
    case 0:
        if (role == Qt::DecorationRole) {
            return item_to_image(item, creature_->gender == 1);
        }
        break;
    case 1:
        if (role == Qt::DisplayRole) {
            return to_qstring(nw::kernel::strings().get(item->common.name));
        }
        break;
    case 2:
        if (role == Qt::DisplayRole) {
            return item->stacksize;
        }
        break;
    }
    return QVariant();
}

Qt::ItemFlags InventoryModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    return flags;
}

void InventoryModel::addItem(nw::Item* item)
{
    if (creature_->inventory.can_add_item(item)) {
        beginInsertRows(QModelIndex(), int(creature_->inventory.items.size()), int(creature_->inventory.items.size()));
        creature_->inventory.add_item(item);
        endInsertRows();
    }
}

nw::Creature* InventoryModel::creature() const noexcept
{
    return creature_;
}

void InventoryModel::removeItem(nw::Item* item)
{
    auto it = std::find_if(creature_->inventory.items.begin(), creature_->inventory.items.end(),
        [item](const nw::InventoryItem& ii) { return ii.item == item; });

    if (it != std::end(creature_->inventory.items)) {
        int index = int(std::distance(creature_->inventory.items.begin(), it));
        beginRemoveRows(QModelIndex(), index, index);
        creature_->inventory.remove_item(item);
        endRemoveRows();
    }
}

// == CreatureInventoryView ===================================================
// ============================================================================

CreatureInventoryView::CreatureInventoryView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureInventoryView)
{
    ui->setupUi(this);
    ui->add->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_plus, Qt::green));
    ui->remove->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_minus, Qt::red));
    connect(ui->remove, &QPushButton::clicked, this, &CreatureInventoryView::onRemove);
}

CreatureInventoryView::~CreatureInventoryView()
{
    delete ui;
}

void CreatureInventoryView::connectSlots(CreatureEquipView* equips)
{
    connect(ui->inventoryView, &InventoryTable::unequipItem, equips, &CreatureEquipView::unequipItem);
}

void CreatureInventoryView::setCreature(nw::Creature* creature)
{
    creature_ = creature;
    model_ = new InventoryModel(creature_, this);
    ui->inventoryView->setModel(model_);
    ui->inventoryView->setItemDelegate(new InventoryItemDelegate(this));
    ui->inventoryView->setColumnWidth(0, 64);
    ui->inventoryView->setColumnWidth(2, 80);

    ui->inventoryView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->inventoryView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->inventoryView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->inventoryView->resizeRowsToContents();
    ui->inventoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void CreatureInventoryView::removeItemFromInventory(nw::Item* item)
{
    CHECK_F(!!item, "item is null");
    model_->removeItem(item);
}

void CreatureInventoryView::addItemToInventory(nw::Item* item)
{
    CHECK_F(!!item, "item is null");
    model_->addItem(item);
    ui->inventoryView->resizeRowsToContents();
}

void CreatureInventoryView::onRemove(bool checked)
{
    Q_UNUSED(checked);
    if (!model_) { return; }

    auto selmodel = ui->inventoryView->selectionModel();
    if (!selmodel) { return; }

    auto indices = selmodel->selectedRows();
    std::sort(indices.rbegin(), indices.rend());

    foreach (const auto& index, indices) {
        if (index.isValid()) {
            auto item = reinterpret_cast<nw::Item*>(index.internalPointer());
            model_->removeItem(item);
            nw::kernel::objects().destroy(item->handle());
        }
    }
}
