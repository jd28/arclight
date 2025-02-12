#include "creaturepropertiestab.h"
#include "ui_creaturepropertiestab.h"

CreaturePropertiesTab::CreaturePropertiesTab(nw::Creature* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::CreaturePropertiesTab)
    , obj_{obj}
{
    ui->setupUi(this);
    ui->properties->setCreature(obj_);
    ui->properties->setUndoStack(undoStack());
}

CreaturePropertiesTab::~CreaturePropertiesTab()
{
    delete ui;
}

CreaturePropertiesView* CreaturePropertiesTab::properties() const noexcept
{
    return ui->properties;
}
