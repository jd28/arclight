#ifndef CREATUREEQUIPVIEW_H
#define CREATUREEQUIPVIEW_H

#include <QWidget>

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureEquipView;
}

class CreatureEquipView : public QWidget {
    Q_OBJECT

public:
    explicit CreatureEquipView(QWidget* parent = nullptr);
    ~CreatureEquipView();

    void setCreature(nw::Creature* creature);

private:
    Ui::CreatureEquipView* ui;
    nw::Creature* creature_ = nullptr;
};

#endif // CREATUREEQUIPVIEW_H
