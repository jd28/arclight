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
    void setCreature(nw::Creature* creature);
    void setItem(nw::Item* item);
    nw::EquipSlot slot() const noexcept;
    void setSlot(nw::EquipSlot slot);

signals:
    void equipItem(nw::Item* item, nw::EquipSlot slot);
    void unequipItem(nw::Item* item, nw::EquipSlot slot);
    void addItemToInventory(nw::Item* item);
    void removeItemFromInventory(nw::Item* item);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    nw::Creature* creature_ = nullptr;
    nw::Item* item_ = nullptr;
    nw::EquipSlot slot_;
    QPixmap default_image_;
    QString default_tip_;

    QPixmap prepareImage(QPixmap image);
};

#endif // INVENTORYSLOT_H
