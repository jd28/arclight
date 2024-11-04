#include "creatureinventorypanel.h"
#include "ui_creatureinventorypanel.h"

CreatureInventoryPanel::CreatureInventoryPanel(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureInventoryPanel)
{
    ui->setupUi(this);
}

CreatureInventoryPanel::~CreatureInventoryPanel()
{
    delete ui;
}

void CreatureInventoryPanel::setCreature(nw::Creature* creature)
{
    ui->equips->setCreature(creature);
    ui->inventory->setCreature(creature);
}
