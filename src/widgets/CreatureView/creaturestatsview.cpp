#include "creaturestatsview.h"
#include "ui_creaturestatsview.h"

#include "../util/strings.h"

#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Creature.hpp"
#include "nw/profiles/nwn1/constants.hpp"
#include "nw/profiles/nwn1/functions.hpp"

CreatureStatsView::CreatureStatsView(nw::Creature* creature, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureStatsView)
    , creature_{creature}
{
    ui->setupUi(this);

    updateAbilities();
    updateArmorClass();
    connect(ui->acNatural, &QSpinBox::valueChanged, this, &CreatureStatsView::onAcNaturalChanged);
    updateSaves();

    QList<QPair<QString, int>> skill_list;
    if (auto skills_2da = nw::kernel::twodas().get("skills")) {
        for (size_t i = 0; i < skills_2da->rows(); ++i) {
            if (auto name_id = skills_2da->get<int32_t>(i, "Name")) {
                auto name = nw::kernel::strings().get(*name_id);
                skill_list.append({to_qstring(name), int(i)});
            }
        }
    }
    std::sort(skill_list.begin(), skill_list.end(), [](const QPair<QString, int>& lhs, const QPair<QString, int>& rhs) {
        return lhs.first < rhs.first;
    });

    int row = 0;
    for (const auto& [name, id] : skill_list) {
        ui->tableWidget->insertRow(row);
        auto x = new QTableWidgetItem(name);
        x->setFlags(x->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 0, x);
        auto y = new QTableWidgetItem(QString::number(nwn1::get_skill_rank(creature_, nw::Skill::make(id))));
        y->setFlags(y->flags() & ~Qt::ItemIsEditable);
        y->setData(Qt::UserRole, id);
        ui->tableWidget->setItem(row, 1, y);
        ++row;
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    updateHitPoints();
    connect(ui->hpBase, &QSpinBox::valueChanged, this, &CreatureStatsView::onHitpointBaseChanged);
}

CreatureStatsView::~CreatureStatsView()
{
    delete ui;
}

void CreatureStatsView::updateAll()
{
    updateAbilities();
    updateArmorClass();
    updateHitPoints();
    updateSaves();
    updateSkills();
}

void CreatureStatsView::updateAbilities()
{
    auto race = nw::kernel::rules().races.get(creature_->race);

    for (int i = 0; i < 6; ++i) {
        int race_mod = 0;
        if (race) { race_mod = race->ability_modifiers[i]; }

        int total = nwn1::get_ability_score(creature_, nw::Ability::make(i));

        auto score = ui->abilityGroup->findChild<QLineEdit*>(QString("ability_%1").arg(i));
        score->setText(QString::number(creature_->stats.abilities_[i]));

        auto race1 = ui->abilityGroup->findChild<QLineEdit*>(QString("abilityRaceLineEdit_%1").arg(i));
        race1->setText(QString::number(race_mod));

        auto bonus = ui->abilityGroup->findChild<QLineEdit*>(QString("abilityBonus_%1").arg(i));
        bonus->setText(QString::number(total - race_mod - creature_->stats.abilities_[i]));

        auto totalw = ui->abilityGroup->findChild<QLineEdit*>(QString("abilityTotalLineEdit_%1").arg(i));
        totalw->setText(QString::number(total));

        auto modifier = ui->abilityGroup->findChild<QLineEdit*>(QString("abilityModLineEdit_%1").arg(i));
        modifier->setText(QString::number(nwn1::get_ability_modifier(creature_, nw::Ability::make(i))));
    }
}

void CreatureStatsView::updateArmorClass()
{
    ui->acNatural->setValue(creature_->combat_info.ac_natural_bonus);
    ui->acSize->setText(QString::number(creature_->combat_info.size_ac_modifier));
    int ac_dex = nwn1::get_dex_modifier(creature_);
    ui->acDexterity->setText(QString::number(ac_dex));
    ui->acTotal->setText(QString::number(nwn1::calculate_ac_versus(creature_, nullptr, false)));
}

void CreatureStatsView::updateHitPoints()
{
    ui->hpBase->setValue(creature_->hp);
    auto hp_max = nwn1::get_max_hitpoints(creature_);
    ui->hpBonuses->setText(QString::number(hp_max - creature_->hp));
    ui->hpTotal->setText(QString::number(hp_max));
}

void CreatureStatsView::updateSaves()
{
    for (auto save : {nwn1::saving_throw_fort, nwn1::saving_throw_reflex, nwn1::saving_throw_will}) {
        int base = 0;
        int bonus = 0;
        int mod = 0;

        // Class
        auto& classes = nw::kernel::rules().classes;
        for (size_t i = 0; i < nw::LevelStats::max_classes; ++i) {
            auto id = creature_->levels.entries[i].id;
            int level = creature_->levels.entries[i].level;

            if (id == nw::Class::invalid()) { break; }
            switch (*save) {
            default:
                break;
            case *nwn1::saving_throw_fort:
                base += classes.get_class_save_bonus(id, level).fort;
                break;
            case *nwn1::saving_throw_reflex:
                base += classes.get_class_save_bonus(id, level).reflex;
                break;
            case *nwn1::saving_throw_will:
                base += classes.get_class_save_bonus(id, level).will;
                break;
            }
        }

        switch (*save) {
        default:
            continue;
        case *nwn1::saving_throw_fort:
            mod = nwn1::get_ability_modifier(creature_, nwn1::ability_constitution, true);
            bonus = creature_->stats.save_bonus.fort;
            break;
        case *nwn1::saving_throw_reflex:
            mod = nwn1::get_ability_modifier(creature_, nwn1::ability_dexterity, true);
            bonus = creature_->stats.save_bonus.reflex;
            break;
        case *nwn1::saving_throw_will:
            mod = nwn1::get_ability_modifier(creature_, nwn1::ability_wisdom, true);
            bonus = creature_->stats.save_bonus.will;
            break;
        }

        auto save_base = ui->savesGroup->findChild<QLineEdit*>(QString("saveBase_%1").arg(*save));
        save_base->setText(QString::number(base));

        auto save_mod = ui->savesGroup->findChild<QLineEdit*>(QString("saveModifier_%1").arg(*save));
        save_mod->setText(QString::number(mod));

        auto save_bonus = ui->savesGroup->findChild<QLineEdit*>(QString("saveBonus_%1").arg(*save));
        save_bonus->setText(QString::number(bonus));

        auto save_total = ui->savesGroup->findChild<QLineEdit*>(QString("saveTotal_%1").arg(*save));
        save_total->setText(QString::number(nwn1::saving_throw(creature_, save)));
    }
}

void CreatureStatsView::updateSkills()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        auto it = ui->tableWidget->item(i, 1);
        auto skill = nw::Skill::make(it->data(Qt::UserRole).toInt());
        it->setText(QString::number(nwn1::get_skill_rank(creature_, skill)));
    }
}

// == Public Slots ============================================================
// ============================================================================

void CreatureStatsView::onAcNaturalChanged(int value)
{
    if (!creature_) { return; }
    creature_->combat_info.ac_natural_bonus = value;
    updateArmorClass();
}

void CreatureStatsView::onHitpointBaseChanged(int value)
{
    if (!creature_) { return; }
    creature_->hp = static_cast<int16_t>(value);
    updateHitPoints();
}
