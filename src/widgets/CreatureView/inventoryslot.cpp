#include "inventoryslot.h"

#include "../util/items.h"

#include "nw/kernel/Strings.hpp"
#include "nw/objects/Item.hpp"

#include <QPainter>

InventorySlot::InventorySlot(QWidget* parent)
    : QLabel(parent)
{
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

void InventorySlot::setItem(nw::Item* item, bool female)
{
    item_ = nullptr;
    if (item) {
        auto img = item_to_image(item, female);
        if (!img.isNull()) {
            item_ = item;
            setPixmap(prepareImage(QPixmap::fromImage(img)));
            setToolTip(QString::fromStdString(nw::kernel::strings().get(item->common.name)));
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
