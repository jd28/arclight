#include "objects.h"

#include "nw/formats/Image.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/objects/Item.hpp"
#include "nw/objects/ObjectHandle.hpp"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QImage>
#include <QPainter>

QImage item_to_image(const nw::Item* item, bool female)
{
    if (!item) { return QImage{}; }
    auto bi = nw::kernel::rules().baseitems.get(item->baseitem);
    if (!bi) { return QImage{}; }
    nw::ItemModelType type = bi->model_type;

    if (type == nw::ItemModelType::simple) {
        auto icon = item->get_icon_by_part();
        if (!icon.valid()) { return QImage{}; }
        QImage image(icon.release(), icon.width(), icon.height(), icon.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
            [](void* bytes) { if (bytes) { free(bytes); } });
        return icon.is_bio_dds() ? image.mirrored() : image;
    } else if (type == nw::ItemModelType::layered) {
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

nw::ObjectHandle deserialize_obj_handle(const QByteArray& data)
{
    QDataStream stream(data);

    quint32 id;
    quint8 type;

    stream >> id;
    stream >> type;

    nw::ObjectHandle res;
    res.id = static_cast<nw::ObjectID>(id);
    res.type = static_cast<nw::ObjectType>(type);
    return res;
}

QByteArray serialize_obj_handle(nw::ObjectHandle hndl)
{
    QByteArray res;
    QDataStream stream(&res, QIODevice::WriteOnly);

    stream << static_cast<quint32>(hndl.id);
    stream << static_cast<quint8>(hndl.type);
    return res;
}
