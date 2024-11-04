#include "creatureinventoryview.h"
#include "ui_creatureinventoryview.h"

#include "../util/items.h"

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include <nw/objects/Creature.hpp>
#include <nw/objects/Item.hpp>

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

    const auto item = creature_->inventory.items[index.row()].item.as<nw::Item*>();

    switch (index.column()) {
    case 0:
        if (role == Qt::DecorationRole) {
            return item_to_image(item, creature_->gender == 1);
        }
        break;
    case 1:
        if (role == Qt::DisplayRole) {
            return QString::fromStdString(nw::kernel::strings().get(item->common.name));
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
    // beginInsertRows(QModelIndex(), items.size(), items.size());
    //        items.append({image, name, stackSize, stackable});
    // endInsertRows();
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
}

CreatureInventoryView::~CreatureInventoryView()
{
    delete ui;
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
