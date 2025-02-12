#include "creatureinventorypanel.h"
#include "ui_creatureinventorypanel.h"

#include "nw/objects/Creature.hpp"

// == CreatureInventoryPanel ==================================================
// ============================================================================

CreatureInventoryPanel::CreatureInventoryPanel(ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::CreatureInventoryPanel)
{
    ui->setupUi(this);
    ui->equips->connectSlots(ui->inventory);
    ui->inventory->connectSlots(ui->equips);
}

CreatureInventoryPanel::~CreatureInventoryPanel()
{
    delete ui;
}

void CreatureInventoryPanel::setCreature(nw::Creature* creature)
{
    ui->equips->setCreature(creature);
    ui->inventory->setObject(creature);
}
