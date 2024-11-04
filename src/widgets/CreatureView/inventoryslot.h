#ifndef INVENTORYSLOT_H
#define INVENTORYSLOT_H

#include <nw/objects/Equips.hpp>

#include <QLabel>
#include <QObject>

class InventorySlot : public QLabel {
    Q_OBJECT
public:
    explicit InventorySlot(QWidget* parent = nullptr);

    void setDefaults(QPixmap image, QString tip);
    nw::Item* item() const noexcept;
    void setItem(nw::Item* item, bool female = false);
    nw::EquipSlot slot() const noexcept;
    void setSlot(nw::EquipSlot slot);

private:
    nw::Item* item_ = nullptr;
    nw::EquipSlot slot_;
    QPixmap default_image_;
    QString default_tip_;

    QPixmap prepareImage(QPixmap image);
};

#endif // INVENTORYSLOT_H
