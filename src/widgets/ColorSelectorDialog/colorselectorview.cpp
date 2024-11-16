#include "colorselectorview.h"
#include "ui_colorselectorview.h"

#include "nw/rules/items.hpp"

ColorSelectorView::ColorSelectorView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ColorSelectorView)
{
    ui->setupUi(this);

    mvpal_cloth = QPixmap(":/resources/images/mvpal_cloth.png");
    mvpal_leather = QPixmap(":/resources/images/mvpal_leather.png");
    mvpal_metal = QPixmap(":/resources/images/mvpal_armor01.png");

    ui->part->addItem("Model", -1);
    ui->part->addItem("Belt", int(nw::ItemModelParts::armor_belt));
    ui->part->addItem("Bicep, Left", int(nw::ItemModelParts::armor_lbicep));
    ui->part->addItem("Bicep, Right", int(nw::ItemModelParts::armor_rbicep));
    ui->part->addItem("Foot, Left", int(nw::ItemModelParts::armor_lfoot));
    ui->part->addItem("Foot, Right", int(nw::ItemModelParts::armor_rfoot));
    ui->part->addItem("Forearm, Left", int(nw::ItemModelParts::armor_lfarm));
    ui->part->addItem("Forearm, Right", int(nw::ItemModelParts::armor_rfarm));
    ui->part->addItem("Hand, Left", int(nw::ItemModelParts::armor_lhand));
    ui->part->addItem("Hand, Right", int(nw::ItemModelParts::armor_rhand));
    ui->part->addItem("Neck", int(nw::ItemModelParts::armor_neck));
    ui->part->addItem("Pelvis", int(nw::ItemModelParts::armor_pelvis));
    ui->part->addItem("Rove", int(nw::ItemModelParts::armor_robe));
    ui->part->addItem("Shin, Left", int(nw::ItemModelParts::armor_lshin));
    ui->part->addItem("Shin, Right", int(nw::ItemModelParts::armor_rshin));
    ui->part->addItem("Shoulder, Left", int(nw::ItemModelParts::armor_lshoul));
    ui->part->addItem("Shoulder, Right", int(nw::ItemModelParts::armor_rshoul));
    ui->part->addItem("Thigh, Left", int(nw::ItemModelParts::armor_lthigh));
    ui->part->addItem("Thigh, Right", int(nw::ItemModelParts::armor_rthigh));
    ui->part->addItem("Torso", int(nw::ItemModelParts::armor_torso));

    ui->color->addItem("Cloth 1", int(nw::ItemColors::cloth1));
    ui->color->addItem("Cloth 2", int(nw::ItemColors::cloth2));
    ui->color->addItem("Leather 1", int(nw::ItemColors::leather1));
    ui->color->addItem("Leather 2", int(nw::ItemColors::leather2));
    ui->color->addItem("Metal 1", int(nw::ItemColors::metal1));
    ui->color->addItem("Metal 2", int(nw::ItemColors::metal2));
}

ColorSelectorView::~ColorSelectorView()
{
    delete ui;
}

QPixmap ColorSelectorView::getPalette(nw::ItemColors::type index) const noexcept
{
    switch (index) {

    case nw::ItemColors::cloth1:
    case nw::ItemColors::cloth2:
        return mvpal_cloth;
    case nw::ItemColors::leather1:
    case nw::ItemColors::leather2:
        return mvpal_leather;
    case nw::ItemColors::metal1:
    case nw::ItemColors::metal2:
        return mvpal_metal;
    }

    return {};
}
