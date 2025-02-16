#include "inventoryview.h"
#include "ui_inventoryview.h"

#include "../CreatureView/creatureequipview.h"
#include "../checkboxdelegate.h"
#include "../projectview.h"
#include "../util/objects.h"
#include "../util/strings.h"

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include "nw/kernel/Objects.hpp"
#include "nw/kernel/Rules.hpp"
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
        auto model = static_cast<InventoryModel*>(this->model());
        if (!model || !model->object()) {
            QTableView::mousePressEvent(event);
            return;
        }

        if (model->object()->as_creature() && dragEnabled()) {
            QModelIndex index = indexAt(event->pos());
            auto idx = model->index(index.row(), 0, index.parent());
            if (index.isValid()) {
                QMimeData* mimeData = new QMimeData();
                nw::Item* item = reinterpret_cast<nw::Item*>(idx.internalPointer());
                mimeData->setData("application/x-inventory-item", serialize_obj_handle(item->handle()));

                QPixmap img = model->data(idx, Qt::DecorationRole).value<QPixmap>();
                QPoint hotspot = QPoint(img.width() / 2, img.height() / 2);

                QDrag* drag = new QDrag(this);
                drag->setMimeData(mimeData);
                drag->setPixmap(img);
                drag->setHotSpot(hotspot);
                drag->exec(Qt::MoveAction);
                return;
            }
        }
    }
    QTableView::mousePressEvent(event);
}

void InventoryTable::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QModelIndex index = indexAt(event->pos());
        auto model = static_cast<InventoryModel*>(this->model());

        if (index.isValid() && index.column() == 3 && model && model->object() && model->object()->as_store()) {
            bool current = model->data(index, Qt::DisplayRole).toBool();
            // InventoryView* view = qobject_cast<InventoryView*>(parent());
            model->setData(index, !current, Qt::EditRole);
            return;
        }
    }

    QTableView::mouseReleaseEvent(event);
}

void InventoryTable::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-equip-item") || event->mimeData()->hasFormat("application/x-arclight-projectitem")) {
        event->acceptProposedAction();
        return;
    }
    QTableView::dragEnterEvent(event);
}

void InventoryTable::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-equip-item") || event->mimeData()->hasFormat("application/x-arclight-projectitem")) {
        event->acceptProposedAction();
        return;
    }
    QTableView::dragMoveEvent(event);
}

void InventoryTable::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-equip-item")) {
        QByteArray itemData = event->mimeData()->data("application/x-equip-item");
        auto item_handle = deserialize_obj_handle(itemData);
        auto item = nw::kernel::objects().get<nw::Item>(item_handle);
        if (!item) {
            event->ignore();
            return;
        }

        auto m = static_cast<InventoryModel*>(model());

        auto cre = m->object() ? m->object()->as_creature() : nullptr;
        if (!cre) {
            event->ignore();
            return;
        }

        int i = 0;
        for (const auto& it : cre->equipment.equips) {
            if (it.is<nw::Item*>() && it.as<nw::Item*>() == item) { break; }
            ++i;
        }

        auto slot = static_cast<nw::EquipSlot>(1 << i);
        emit unequipItem(item, slot);

        m->addItem(item);
        resizeRowsToContents();
        event->acceptProposedAction();
    } else if (event->mimeData()->hasFormat("application/x-arclight-projectitem")) {
        QByteArray data_ = event->mimeData()->data("application/x-arclight-projectitem");
        QDataStream stream(&data_, QIODevice::ReadOnly);
        qint64 senderPid;
        stream >> senderPid;
        if (senderPid != QCoreApplication::applicationPid()) {
            event->ignore();
            return;
        }

        qlonglong ptr;
        stream >> ptr;
        const ProjectItem* node = reinterpret_cast<const ProjectItem*>(ptr);
        if (!node) { return; }
        if (node->res_.type != nw::ResourceType::uti) {
            event->ignore();
            return;
        }

        auto m = static_cast<InventoryModel*>(model());
        auto item = nw::kernel::objects().load<nw::Item>(node->res_.resref);
        if (!item) {
            event->ignore();
            return;
        }

        if (m->object()->as_store()) {
            auto bi_info = nw::kernel::rules().baseitems.get(item->baseitem);
            if (!bi_info || !bi_info->valid()) {
                nw::kernel::objects().destroy(item->handle());
                event->ignore();
                return;
            }
            if (bi_info->store_panel != m->store_tab()) {
                nw::kernel::objects().destroy(item->handle());
                event->ignore();
                return;
            }
        }

        m->addItem(item);
        resizeRowsToContents();
        event->acceptProposedAction();
        return;
    }
    event->ignore();
    QTableView::dropEvent(event);
}

// == InventoryItemDelegate ===================================================
// ============================================================================

InventoryItemDelegate::InventoryItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
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

InventoryModel::InventoryModel(nw::ObjectBase* obj, nw::Inventory* inventory, int store_tab, QObject* parent)
    : QAbstractTableModel(parent)
    , obj_{obj}
    , inventory_{inventory}
    , store_tab_{store_tab}
{
}

void InventoryModel::addItem(nw::Item* item)
{
    if (inventory_->can_add_item(item)) {
        beginInsertRows(QModelIndex(), int(inventory_->items.size()), int(inventory_->items.size()));
        inventory_->add_item(item);
        endInsertRows();
    }
}

nw::ObjectBase* InventoryModel::object() const noexcept
{
    return obj_;
}

void InventoryModel::removeItem(nw::Item* item)
{
    auto it = std::find_if(inventory_->items.begin(), inventory_->items.end(),
        [item](const nw::InventoryItem& ii) { return ii.item == item; });

    if (it != std::end(inventory_->items)) {
        int index = int(std::distance(inventory_->items.begin(), it));
        beginRemoveRows(QModelIndex(), index, index);
        inventory_->remove_item(item);
        endRemoveRows();
    }
}

int InventoryModel::store_tab() const noexcept
{
    return store_tab_;
}

bool InventoryModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if (data->hasFormat("application/x-equip-item")) {
        return true;
    } else if (data->hasFormat("application/x-arclight-projectitem")) {
        QByteArray data_ = data->data("application/x-arclight-projectitem");
        QDataStream stream(&data_, QIODevice::ReadOnly);
        qint64 senderPid;
        stream >> senderPid;
        if (senderPid != QCoreApplication::applicationPid()) {
            return false;
        }

        qlonglong ptr;
        stream >> ptr;
        const ProjectItem* node = reinterpret_cast<const ProjectItem*>(ptr);
        if (!node) { return false; }

        if (node->res_.type == nw::ResourceType::uti) {
            if (store_tab_ == -1) { return true; }
            auto item = nw::kernel::objects().load<nw::Item>(node->res_.resref);
            if (!item) { return false; }
            auto bi_info = nw::kernel::rules().baseitems.get(item->baseitem);
            if (!bi_info || !bi_info->valid()) { return false; }
            return bi_info->store_panel == store_tab_;
        }
    }
    return false;
}

int InventoryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return obj_->as_store() ? 4 : 3;
}

QVariant InventoryModel::data(const QModelIndex& index, int role) const
{
    if (!obj_ || !inventory_) { return {}; }
    if (!index.isValid() || index.row() >= static_cast<int>(inventory_->items.size())) {
        return {};
    }

    if (!inventory_->items[index.row()].item.is<nw::Item*>()) {
        return {};
    }

    const auto item = inventory_->items[index.row()].item.as<nw::Item*>();
    if (!item) { return {}; }

    switch (index.column()) {
    case 0:
        if (role == Qt::DecorationRole) {
            auto cre = obj_->as_creature();
            return item_to_image(item, cre ? cre->gender == 1 : false);
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
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        }
        break;
    case 3:
        if (role == Qt::DisplayRole) {
            return inventory_->items[index.row()].infinite;
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
        case 3:
            return "Infinite";
        default:
            return QVariant();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QModelIndex InventoryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) { return {}; }
    if (!obj_ || !inventory_) { return {}; }

    nw::Item* item = inventory_->items[row].item.as<nw::Item*>();
    if (!item) { return {}; }

    return createIndex(row, column, item);
}

int InventoryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if (!obj_ || !inventory_) { return 0; }
    return static_cast<int>(inventory_->items.size());
}

bool InventoryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || !obj_ || !inventory_ || index.row() >= static_cast<int>(inventory_->items.size())) {
        return false;
    }

    if (role == Qt::EditRole && index.column() == 3 && obj_->as_store()) {
        inventory_->items[index.row()].infinite = value.toBool();
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

Qt::DropActions InventoryModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

// == InventoryView ===========================================================
// ============================================================================

InventoryView::InventoryView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::InventoryView)
{
    ui->setupUi(this);
    ui->add->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_plus, Qt::green));
    ui->remove->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_minus, Qt::red));
    connect(ui->remove, &QPushButton::clicked, this, &InventoryView::onRemove);
}

InventoryView::~InventoryView()
{
    delete ui;
}

void InventoryView::connectSlots(CreatureEquipView* equips)
{
    connect(ui->inventoryView, &InventoryTable::unequipItem, equips, &CreatureEquipView::unequipItem);
}

void InventoryView::setDragEnabled(bool value)
{
    ui->inventoryView->setDragEnabled(value);
}

void InventoryView::setObject(nw::ObjectBase* obj, nw::Inventory* inventory, int store_tab)
{
    obj_ = obj;
    model_ = new InventoryModel(obj_, inventory, store_tab, this);
    ui->inventoryView->setModel(model_);
    ui->inventoryView->setItemDelegateForColumn(0, new InventoryItemDelegate(this));
    if (obj_->as_store()) {
        ui->inventoryView->setItemDelegateForColumn(3, new CheckBoxDelegate(this));
    }
    ui->inventoryView->setColumnWidth(0, 64);
    ui->inventoryView->setColumnWidth(2, 80);

    ui->inventoryView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->inventoryView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->inventoryView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->inventoryView->resizeRowsToContents();
    ui->inventoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void InventoryView::removeItemFromInventory(nw::Item* item)
{
    CHECK_F(!!item, "item is null");
    model_->removeItem(item);
}

void InventoryView::addItemToInventory(nw::Item* item)
{
    CHECK_F(!!item, "item is null");
    model_->addItem(item);
    ui->inventoryView->resizeRowsToContents();
}

void InventoryView::onRemove(bool checked)
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
