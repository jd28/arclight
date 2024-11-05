#include "creatureinventorypanel.h"
#include "ui_creatureinventorypanel.h"

CreatureInventoryPanel::CreatureInventoryPanel(QWidget* parent)
    : QWidget(parent)
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
    ui->inventory->setCreature(creature);
}
