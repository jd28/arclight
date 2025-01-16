#include "inventoryslot.h"

#include "../util/objects.h"
#include "../util/strings.h"

#include "nw/kernel/Objects.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/objects/Item.hpp"
#include "nw/profiles/nwn1/scriptapi.hpp"

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

InventorySlot::InventorySlot(QWidget* parent)
    : QLabel(parent)
{
    this->setAcceptDrops(true);
}

void InventorySlot::dragEnterEvent(QDragEnterEvent* event)
{
    QByteArray ba;

    if (event->mimeData()->hasFormat("application/x-inventory-item")) {
        ba = event->mimeData()->data("application/x-inventory-item");
    } else if (event->mimeData()->hasFormat("application/x-equip-item")) {
        ba = event->mimeData()->data("application/x-equip-item");
    }
    if (ba.size() == 0) { return; }

    auto item_handle = deserialize_obj_handle(ba);
    nw::Item* item = nw::kernel::objects().get<nw::Item>(item_handle);
    if (!item || item == item_) { return; }
    if (nwn1::can_equip_item(creature_, item, nw::equip_slot_to_index(slot_))) {
        event->acceptProposedAction();
    }
}

void InventorySlot::dropEvent(QDropEvent* event)
{
    QByteArray ba;
    bool inv = false;
    if (event->mimeData()->hasFormat("application/x-inventory-item")) {
        ba = event->mimeData()->data("application/x-inventory-item");
        inv = true;
    } else if (event->mimeData()->hasFormat("application/x-equip-item")) {
        ba = event->mimeData()->data("application/x-equip-item");
    }
    if (ba.size() == 0) { return; }

    auto item_handle = deserialize_obj_handle(ba);
    auto item = nw::kernel::objects().get<nw::Item>(item_handle);
    if (!item || item == item_) { return; }

    if (item_) {
        if (inv) { emit addItemToInventory(item_); }
        emit unequipItem(item_, slot_);
    }

    if (inv) { emit removeItemFromInventory(item); }
    emit equipItem(item, slot_);
    event->acceptProposedAction();
    setItem(item);
}

void InventorySlot::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && item_) {
        QMimeData* mimeData = new QMimeData();
        // Optionally, include item data such as item ID or index
        mimeData->setData("application/x-equip-item", serialize_obj_handle(item_->handle()));

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setHotSpot(event->pos());
        drag->setPixmap(pixmap());
        drag->exec(Qt::MoveAction);
    }
}

void InventorySlot::setDefaults(QPixmap image, QString tip)
{
    default_tip_ = std::move(tip);
    default_image_ = prepareImage(std::move(image));
    if (!item_) {
        setPixmap(default_image_);
        setToolTip(default_tip_);
    }
}

nw::Item* InventorySlot::item() const noexcept
{
    return item_;
}

void InventorySlot::setCreature(nw::Creature* creature)
{
    creature_ = creature;
}

void InventorySlot::setItem(nw::Item* item)
{
    item_ = nullptr;
    if (item) {
        auto img = item_to_image(item, creature_->gender == 1);
        if (!img.isNull()) {
            item_ = item;
            setPixmap(prepareImage(QPixmap::fromImage(img)));
            setToolTip(to_qstring(nw::kernel::strings().get(item->common.name)));
        }
    }
    if (!item_) {
        setPixmap(default_image_);
        setToolTip(default_tip_);
    }
}

nw::EquipSlot InventorySlot::slot() const noexcept
{
    return slot_;
}

void InventorySlot::setSlot(nw::EquipSlot slot)
{
    slot_ = slot;
}

QPixmap InventorySlot::prepareImage(QPixmap image)
{
    if (slot_ == nw::EquipSlot::righthand) {
        //        image = image.copy(0, 0, image.width(), image.height() - 16);
    } else if (slot_ == nw::EquipSlot::chest || slot_ == nw::EquipSlot::cloak) {
        image = image.copy(0, 0, image.width(), image.height() - 8);
    }
    return image;
}
