#include "creaturestatsview.h"
#include "ui_creaturestatsview.h"

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

    for (int i = 0; i < 6; ++i) {
        auto score = ui->abilityGroup->findChild<QSpinBox*>(QString("abilitySpinBox_%1").arg(i));
        score->setProperty("ability", i);
        connect(score, &QSpinBox::valueChanged, this, &CreatureStatsView::onAbilityScoreChanged);
    }

    updateAbilities();
    updateArmorClass();
    connect(ui->acNatural, &QSpinBox::valueChanged, this, &CreatureStatsView::onAcNaturalChanged);
    updateSaves();

    QList<QPair<QString, int>> skill_list;
    if (auto skills_2da = nw::kernel::twodas().get("skills")) {
        for (size_t i = 0; i < skills_2da->rows(); ++i) {
            if (auto name_id = skills_2da->get<int32_t>(i, "Name")) {
                auto name = nw::kernel::strings().get(*name_id);
                skill_list.append({QString::fromStdString(name), int(i)});
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
        auto y = new QTableWidgetItem(QString::number(nwn1::get_skill_rank(creature_, nw::Skill::make(id), nullptr, true)));
        y->setData(Qt::UserRole, id);
        ui->tableWidget->setItem(row, 1, y);
        ++row;
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &CreatureStatsView::onSkillChanged);

    auto ranges_2da = nw::kernel::twodas().get("ranges");
    int ranges_idx = -1;
    if (ranges_2da) {
        for (size_t i = 0; i < ranges_2da->rows(); ++i) {
            int name;
            if (ranges_2da->get_to(i, "Name", name)) {
                auto string = nw::kernel::strings().get(uint32_t(name));
                ui->perceptionCombo->addItem(QString::fromStdString(string), int(i));
                if (i == creature_->perception_range) {
                    ranges_idx = ui->perceptionCombo->count() - 1;
                }
            }
        }
    }
    if (ranges_idx != -1) { ui->perceptionCombo->setCurrentIndex(ranges_idx); }
    ui->perceptionCombo->model()->sort(0);
    connect(ui->perceptionCombo, &QComboBox::currentIndexChanged, this, &CreatureStatsView::onPerceptionChanged);

    auto creaturespeed_2da = nw::kernel::twodas().get("creaturespeed");
    int creaturespeed_idx = -1;
    if (creaturespeed_2da) {
        for (size_t i = 0; i < creaturespeed_2da->rows(); ++i) {
            int name;
            if (creaturespeed_2da->get_to(i, "Name", name)) {
                auto string = nw::kernel::strings().get(uint32_t(name));
                ui->moveRateCombo->addItem(QString::fromStdString(string), int(i));
                if (int32_t(i) == creature_->walkrate) {
                    creaturespeed_idx = ui->moveRateCombo->count() - 1;
                }
            }
        }
    }
    if (creaturespeed_idx != -1) { ui->moveRateCombo->setCurrentIndex(creaturespeed_idx); }
    ui->moveRateCombo->model()->sort(0);
    connect(ui->moveRateCombo, &QComboBox::currentIndexChanged, this, &CreatureStatsView::onMoveRateChanged);

    updateHitPoints();
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
}

void CreatureStatsView::updateAbilities()
{
    auto race = nw::kernel::rules().races.get(creature_->race);

    for (int i = 0; i < 6; ++i) {
        int base = nwn1::get_ability_score(creature_, nw::Ability::make(i), true);
        int mod = nwn1::get_ability_modifier(creature_, nw::Ability::make(i), true);
        int race_mod = 0;
        if (race) { race_mod = race->ability_modifiers[i]; }

        auto score = ui->abilityGroup->findChild<QSpinBox*>(QString("abilitySpinBox_%1").arg(i));
        score->setValue(base - race_mod);

        auto race1 = ui->abilityGroup->findChild<QLineEdit*>(QString("abilityRaceLineEdit_%1").arg(i));
        race1->setText(QString::number(race_mod));

        auto total = ui->abilityGroup->findChild<QLineEdit*>(QString("abilityTotalLineEdit_%1").arg(i));
        total->setText(QString::number(base));

        auto modifier = ui->abilityGroup->findChild<QLineEdit*>(QString("abilityModLineEdit_%1").arg(i));
        modifier->setText(QString::number(mod));
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
    for (auto skill : {nwn1::saving_throw_fort, nwn1::saving_throw_reflex, nwn1::saving_throw_will}) {
        int base = 0;
        int bonus = 0;
        int mod = 0;

        // Class
        auto& classes = nw::kernel::rules().classes;
        for (size_t i = 0; i < nw::LevelStats::max_classes; ++i) {
            auto id = creature_->levels.entries[i].id;
            int level = creature_->levels.entries[i].level;

            if (id == nw::Class::invalid()) { break; }
            switch (*skill) {
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

        switch (*skill) {
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

        auto save_base = ui->savesGroup->findChild<QLineEdit*>(QString("saveBase_%1").arg(*skill));
        save_base->setText(QString::number(base));

        auto save_mod = ui->savesGroup->findChild<QLineEdit*>(QString("saveModifier_%1").arg(*skill));
        save_mod->setText(QString::number(mod));

        auto save_bonus = ui->savesGroup->findChild<QSpinBox*>(QString("savesBonus_%1").arg(*skill));
        save_bonus->setValue(bonus);

        auto save_total = ui->savesGroup->findChild<QLineEdit*>(QString("saveTotal_%1").arg(*skill));
        save_total->setText(QString::number(base + mod + bonus));
    }
}

// == Public Slots ============================================================
// ============================================================================

void CreatureStatsView::onAbilityScoreChanged(int value)
{
    auto score = qobject_cast<QSpinBox*>(sender());
    auto abil = score->property("ability").toInt();
    creature_->stats.set_ability_score(nw::Ability::make(abil), value);

    updateAll();
}

void CreatureStatsView::onAcNaturalChanged(int value)
{
    Q_UNUSED(value);
    if (!creature_) { return; }
    creature_->combat_info.ac_natural_bonus = ui->acNatural->value();
    updateArmorClass();
}

void CreatureStatsView::onMoveRateChanged(int idx)
{
    Q_UNUSED(idx);
    if (!creature_) { return; }

    auto val = ui->moveRateCombo->currentData().toInt();
    creature_->walkrate = val;
}

void CreatureStatsView::onPerceptionChanged(int idx)
{
    Q_UNUSED(idx);
    if (!creature_) { return; }

    auto val = ui->perceptionCombo->currentData().toInt();
    creature_->perception_range = uint8_t(val);
}

void CreatureStatsView::onSkillChanged(int row, int col)
{
    if (col != 1) { return; }
    auto item = ui->tableWidget->item(row, col);
    auto skill = nw::Skill::make(item->data(Qt::UserRole).toInt());
    auto value = item->data(Qt::DisplayRole).toInt();
    creature_->stats.set_skill_rank(skill, value);
    updateAll();
}
