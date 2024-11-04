#include "creatureequipview.h"
#include "ui_creatureequipview.h"

#include "nw/objects/Creature.hpp"
#include "nw/objects/Item.hpp"

#include <QByteArray>
#include <QImage>
#include <QPainter>

CreatureEquipView::CreatureEquipView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureEquipView)
{
    ui->setupUi(this);

    ui->slotRight->setSlot(nw::EquipSlot::righthand);
    ui->slotRight->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_right.png")), "Right Hand");

    ui->slotArmor->setSlot(nw::EquipSlot::chest);
    ui->slotArmor->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_armor.png")), "Armor");

    ui->slotArrow->setSlot(nw::EquipSlot::arrows);
    ui->slotArrow->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_arrow.png")), "Arrows");

    ui->slotSling->setSlot(nw::EquipSlot::bullets);
    ui->slotSling->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_sling.png")), "Bullets");

    ui->slotBolt->setSlot(nw::EquipSlot::bolts);
    ui->slotBolt->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_bolts.png")), "Bolts");

    ui->slotLeft->setSlot(nw::EquipSlot::lefthand);
    ui->slotLeft->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_left.png")), "Left Hand");

    ui->slotBelt->setSlot(nw::EquipSlot::belt);
    ui->slotBelt->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_belt.png")), "Belt");

    ui->slotHelmet->setSlot(nw::EquipSlot::head);
    ui->slotHelmet->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_helm.png")), "Helmet");

    ui->slotGloves->setSlot(nw::EquipSlot::arms);
    ui->slotGloves->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_gloves.png")), "Gloves");

    ui->slotRing1->setSlot(nw::EquipSlot::rightring);
    ui->slotRing1->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_ring.png")), "Ring");

    ui->slotRing2->setSlot(nw::EquipSlot::leftring);
    ui->slotRing2->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_ring.png")), "Ring");

    ui->slotAmulet->setSlot(nw::EquipSlot::neck);
    ui->slotAmulet->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_amulet.png")), "Amulet");

    ui->slotCloak->setSlot(nw::EquipSlot::cloak);
    ui->slotCloak->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_cloak.png")), "Cloak");

    ui->slotBoots->setSlot(nw::EquipSlot::boots);
    ui->slotBoots->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_boots.png")), "Boots");

    ui->slotCreature1->setSlot(nw::EquipSlot::creature_right);
    ui->slotCreature1->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_cre_1.png")), "Attack 1");

    ui->slotCreature2->setSlot(nw::EquipSlot::creature_left);
    ui->slotCreature2->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_cre_2.png")), "Attack 2");

    ui->slotCreature3->setSlot(nw::EquipSlot::creature_bite);
    ui->slotCreature3->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_cre_3.png")), "Special Attack");

    ui->slotCreatureSkin->setSlot(nw::EquipSlot::creature_skin);
    ui->slotCreatureSkin->setDefaults(QPixmap(QString::fromUtf8(":/resources/images/inv_slot_cre_skin.png")), "Skin");
}

CreatureEquipView::~CreatureEquipView()
{
    delete ui;
}

void CreatureEquipView::setCreature(nw::Creature* creature)
{
    for (size_t i = 0; i < 18; ++i) {
        nw::EquipIndex idx = static_cast<nw::EquipIndex>(i);

        if (creature->equipment.equips[i].is<nw::Item*>()) {
            auto it = creature->equipment.equips[i].as<nw::Item*>();
            switch (idx) {
            default:
                break;

            case nw::EquipIndex::head:
                ui->slotHelmet->setItem(it);
                break;
            case nw::EquipIndex::chest:
                ui->slotArmor->setItem(it);
                break;
            case nw::EquipIndex::boots:
                ui->slotBoots->setItem(it);
                break;
            case nw::EquipIndex::arms:
                ui->slotGloves->setItem(it);
                break;
            case nw::EquipIndex::righthand:
                ui->slotRight->setItem(it);
                break;
            case nw::EquipIndex::lefthand:
                ui->slotLeft->setItem(it);
                break;
            case nw::EquipIndex::cloak:
                ui->slotCloak->setItem(it);
                break;
            case nw::EquipIndex::leftring:
                ui->slotRing2->setItem(it);
                break;
            case nw::EquipIndex::rightring:
                ui->slotRing1->setItem(it);
                break;
            case nw::EquipIndex::neck:
                ui->slotAmulet->setItem(it);
                break;
            case nw::EquipIndex::belt:
                ui->slotBolt->setItem(it);
                break;
            case nw::EquipIndex::arrows:
                ui->slotArrow->setItem(it);
                break;
            case nw::EquipIndex::bullets:
                ui->slotSling->setItem(it);
                break;
            case nw::EquipIndex::bolts:
                ui->slotBolt->setItem(it);
                break;
            case nw::EquipIndex::creature_left:
                ui->slotCreature1->setItem(it);
                break;
            case nw::EquipIndex::creature_right:
                ui->slotCreature2->setItem(it);
                break;
            case nw::EquipIndex::creature_bite:
                ui->slotCreature3->setItem(it);
                break;
            case nw::EquipIndex::creature_skin:
                ui->slotCreatureSkin->setItem(it);
                break;
            }
        }
    }

    creature_ = creature;
}
