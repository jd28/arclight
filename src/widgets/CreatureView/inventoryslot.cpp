#include "inventoryslot.h"

#include "nw/formats/Image.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Item.hpp"

#include <QPainter>

QImage item_to_image(const nw::Item* item, bool female)
{
    auto* bi2da = nw::kernel::twodas().get("baseitems");
    auto int_type = bi2da->get<int32_t>(*item->baseitem, "ModelType");
    nw::ItemModelType type;

    if (int_type) {
        type = static_cast<nw::ItemModelType>(*int_type);
    } else {
        return QImage();
    }

    if (type == nw::ItemModelType::simple || type == nw::ItemModelType::layered) {
        auto icon = item->get_icon_by_part();
        if (!icon.valid()) { return QImage{}; }
        QImage image(icon.release(), icon.width(), icon.height(), icon.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
            [](void* bytes) { if (bytes) { free(bytes); } });
        return image.mirrored();
    } else if (type == nw::ItemModelType::composite) {
        auto img1 = item->get_icon_by_part(nw::ItemModelParts::model1);
        if (!img1.valid()) { return QImage{}; }
        QImage bottom(img1.release(), img1.width(), img1.height(), img1.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
            [](void* bytes) { if (bytes) { free(bytes); } });
        if (!img1.is_bio_dds()) { bottom.mirror(); }

        auto img2 = item->get_icon_by_part(nw::ItemModelParts::model2);
        if (!img2.valid()) { return QImage{}; }
        QImage middle(img2.release(), img2.width(), img2.height(), img2.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
            [](void* bytes) { if (bytes) { free(bytes); } });
        if (!img2.is_bio_dds()) { middle.mirror(); }

        auto img3 = item->get_icon_by_part(nw::ItemModelParts::model3);
        if (!img3.valid()) { return QImage{}; }
        QImage top(img3.release(), img3.width(), img3.height(), img3.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
            [](void* bytes) { if (bytes) { free(bytes); } });
        if (!img3.is_bio_dds()) { top.mirror(); }

        QPainter painter(&bottom);
        painter.drawImage(0, 0, middle);
        painter.drawImage(0, 0, top);
        painter.end();
        return bottom.mirrored();

    } else if (type == nw::ItemModelType::armor) {
        QImage base;

        for (auto part : {
                 nw::ItemModelParts::armor_pelvis,
                 nw::ItemModelParts::armor_belt,
                 nw::ItemModelParts::armor_torso,
                 nw::ItemModelParts::armor_lshoul,
                 nw::ItemModelParts::armor_rshoul,
                 nw::ItemModelParts::armor_robe}) {

            if (item->model_parts[part] == 0) { continue; }
            auto texture = item->get_icon_by_part(part, female);

            if (texture.valid()) {
                QImage image(texture.release(), texture.width(), texture.height(), texture.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
                    [](void* bytes) { if (bytes) { free(bytes); } });
                if (base.isNull()) {
                    base = image;
                } else {
                    QPainter painter(&base);
                    painter.drawImage(0, 0, image);
                }
            }
        }

        if (!base.isNull()) {
            return base.mirrored();
        }
    }

    return QImage();
}

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
