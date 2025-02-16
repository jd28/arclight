#include "storeinventoryview.h"
#include "ui_storeinventoryview.h"

#include "nw/objects/Store.hpp"

StoreInventoryView::StoreInventoryView(nw::Store* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::StoreInventoryView)
    , obj_{obj}
{
    ui->setupUi(this);
    tabs_.push_back(new InventoryView(this));
    tabs_.back()->setObject(obj_, &obj_->inventory.armor, 0);
    ui->tabWidget->addTab(tabs_.back(), "Armor");

    tabs_.push_back(new InventoryView(this));
    tabs_.back()->setObject(obj_, &obj_->inventory.weapons, 1);
    ui->tabWidget->addTab(tabs_.back(), "Weapons");

    tabs_.push_back(new InventoryView(this));
    tabs_.back()->setObject(obj_, &obj_->inventory.potions, 2);
    ui->tabWidget->addTab(tabs_.back(), "Potions/Scrolls");

    tabs_.push_back(new InventoryView(this));
    tabs_.back()->setObject(obj_, &obj_->inventory.rings, 3);
    ui->tabWidget->addTab(tabs_.back(), "Amulets/Rings");

    tabs_.push_back(new InventoryView(this));
    tabs_.back()->setObject(obj_, &obj_->inventory.miscellaneous, 4);
    ui->tabWidget->addTab(tabs_.back(), "Miscellaneous");
}

StoreInventoryView::~StoreInventoryView()
{
    delete ui;
}
