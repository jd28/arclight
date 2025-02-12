#pragma once

#include "../arclighttab.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Creature;
enum struct EquipSlot;
struct Item;
}

namespace Ui {
class CreatureInventoryPanel;
}

// == CreatureInventoryPanel ==================================================
// ============================================================================

class CreatureInventoryPanel : public ArclightTab {
    Q_OBJECT

public:
    explicit CreatureInventoryPanel(ArclightView* parent = nullptr);
    ~CreatureInventoryPanel();

    void setCreature(nw::Creature* creature);

private:
    Ui::CreatureInventoryPanel* ui;
};
