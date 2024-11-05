#ifndef CREATUREEQUIPVIEW_H
#define CREATUREEQUIPVIEW_H

#include <QWidget>

namespace nw {
struct Creature;
enum struct EquipSlot;
struct Item;
}

class CreatureInventoryView;

namespace Ui {
class CreatureEquipView;
}

class CreatureEquipView : public QWidget {
    Q_OBJECT

public:
    explicit CreatureEquipView(QWidget* parent = nullptr);
    ~CreatureEquipView();

    void connectSlots(CreatureInventoryView* inventory);
    void setCreature(nw::Creature* creature);
    void updateEquips();

public slots:
    void equipItem(nw::Item* item, nw::EquipSlot slot);
    void unequipItem(nw::Item* item, nw::EquipSlot slot);

private:
    Ui::CreatureEquipView* ui;
    nw::Creature* creature_ = nullptr;
};

#endif // CREATUREEQUIPVIEW_H
