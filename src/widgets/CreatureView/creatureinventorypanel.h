#ifndef CREATUREINVENTORYPANEL_H
#define CREATUREINVENTORYPANEL_H

#include <QWidget>

namespace nw {
struct Creature;
enum struct EquipSlot;
struct Item;
}

namespace Ui {
class CreatureInventoryPanel;
}

class CreatureInventoryPanel : public QWidget {
    Q_OBJECT

public:
    explicit CreatureInventoryPanel(QWidget* parent = nullptr);
    ~CreatureInventoryPanel();

    void setCreature(nw::Creature* creature);

private:
    Ui::CreatureInventoryPanel* ui;
};

#endif // CREATUREINVENTORYPANEL_H
