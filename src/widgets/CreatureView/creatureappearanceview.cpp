#include "creatureappearanceview.h"
#include "ui_creatureappearanceview.h"

#include "../ColorSelectorDialog/creaturecolorselectordialog.h"
#include "../ColorSelectorDialog/creaturecolorselectorview.h"
#include "../arclight/toolsetservice.h"
#include "../util/strings.h"

#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Creature.hpp"

#include <QAbstractItemView>

CreatureAppearanceView::CreatureAppearanceView(nw::Creature* creature, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureAppearanceView)
{
    ui->setupUi(this);
    mvpal_hair = QPixmap(":/resources/images/mvpal_hair.png");
    mvpal_skin = QPixmap(":/resources/images/mvpal_skin.png");

    int idx = 0;
    std::string temp;

    auto& appearances = nw::kernel::rules().appearances;
    for (size_t i = 0; i < appearances.entries.size(); ++i) {
        if (!appearances.entries[i].valid()) { continue; }

        std::string string;
        if (appearances.entries[i].string_ref != 0xFFFFFFFF) {
            string = nw::kernel::strings().get(appearances.entries[i].string_ref);
            if (!string.empty() && appearances.entries[i].model.size() <= 1) {
                string = fmt::format("(Dynamic) {}", string);
            }
        }

        if (appearances.entries[i].label.empty()) { continue; }

        string = appearances.entries[i].label;

        ui->appearance->addItem(to_qstring(string), int(i));
        if (creature->appearance.id == i) {
            ui->appearance->setCurrentIndex(idx);
            is_dynamic_ = appearances.entries[i].model.size() <= 1;
        }
        ++idx;
    }
    ui->appearance->model()->sort(0, Qt::AscendingOrder);

    auto& phenotypes = nw::kernel::rules().phenotypes;
    idx = 0;
    for (size_t i = 0; i < phenotypes.entries.size(); ++i) {
        if (!phenotypes.entries[i].valid()) { continue; }

        auto string = nw::kernel::strings().get(phenotypes.entries[i].name_ref);
        if (string.empty()) { continue; }

        ui->phenotype->addItem(to_qstring(string), int(i));
        if (creature->appearance.phenotype == int(i)) {
            ui->phenotype->setCurrentIndex(idx);
        }
        ++idx;
    }
    ui->phenotype->model()->sort(0, Qt::AscendingOrder);

    if (is_dynamic_) {
        ui->phenotype->setDisabled(false);
    } else {
        ui->phenotype->setDisabled(true);
        ui->phenotype->setPlaceholderText("-- None --");
        ui->phenotype->setCurrentIndex(-1);
    }

    auto wingmodel_2da = nw::kernel::twodas().get("wingmodel");
    if (!wingmodel_2da) {
        throw std::runtime_error("Unable to load wingmodel.2da");
    }

    ui->wings->addItem("-- None --", 0);
    idx = 1;
    for (size_t i = 1; i < wingmodel_2da->rows(); ++i) {
        if (!wingmodel_2da->get_to(i, "LABEL", temp) || temp.empty()) { continue; }
        ui->wings->addItem(to_qstring(temp), int(i));
        if (creature->appearance.wings == uint32_t(idx)) {
            ui->wings->setCurrentIndex(idx);
        }
        ++idx;
    }
    ui->wings->model()->sort(0, Qt::AscendingOrder);

    auto tailmodel_2da = nw::kernel::twodas().get("tailmodel");
    if (!tailmodel_2da) {
        throw std::runtime_error("Unable to load tailmodel.2da");
    }

    ui->tails->addItem("-- None --", 0);
    idx = 1;
    for (size_t i = 1; i < tailmodel_2da->rows(); ++i) {
        if (!tailmodel_2da->get_to(i, "LABEL", temp) || temp.empty()) { continue; }
        ui->tails->addItem(to_qstring(temp), int(i));
        if (creature->appearance.tail == uint32_t(idx)) {
            ui->tails->setCurrentIndex(idx);
        }
        ++idx;
    }
    ui->tails->model()->sort(0, Qt::AscendingOrder);

    ui->appearance->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    connect(ui->appearance, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onAppearanceChange);
    connect(ui->phenotype, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onPhenotypeChanged);

    connect(ui->hair, &QPushButton::clicked, this, &CreatureAppearanceView::onOpenColorSelector);
    connect(ui->skin, &QPushButton::clicked, this, &CreatureAppearanceView::onOpenColorSelector);
    connect(ui->tatoo1, &QPushButton::clicked, this, &CreatureAppearanceView::onOpenColorSelector);
    connect(ui->tatoo2, &QPushButton::clicked, this, &CreatureAppearanceView::onOpenColorSelector);

    creature_ = creature;

    loadBodyParts();

    connect(ui->bicep_left, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->bicep_right, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->torso, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->foot_left, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->foot_right, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->forearm_left, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->forearm_right, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->hand_left, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->hand_right, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->head, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->thigh_left, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->thigh_right, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->neck, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->pelvis, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->shin_left, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);
    connect(ui->shin_right, &QComboBox::currentIndexChanged, this, &CreatureAppearanceView::onBodyPartChanged);

    if (is_dynamic_) {
        ui->hair->setIcon(getPixmapIcon(nw::CreatureColors::hair));
        ui->skin->setIcon(getPixmapIcon(nw::CreatureColors::skin));
        ui->tatoo1->setIcon(getPixmapIcon(nw::CreatureColors::tatoo1));
        ui->tatoo2->setIcon(getPixmapIcon(nw::CreatureColors::tatoo2));
    }
    updateEnabled();
}

CreatureAppearanceView::~CreatureAppearanceView()
{
    delete ui;
}

void CreatureAppearanceView::loadBodyParts()
{
    auto tool = toolset();
    auto appearance = nw::kernel::rules().appearances.get(nw::Appearance::make(creature_->appearance.id));
    if (!appearance) {
        LOG_F(ERROR, "Invalid appearance");
        return;
    }
    auto phenotype = nw::kernel::rules().phenotypes.get(nw::Phenotype::make(creature_->appearance.phenotype));
    if (!phenotype) {
        LOG_F(ERROR, "Invalid phenotype");
        return;
    }

    QSet<int> model_numbers;
    std::string_view prev;

    for (auto& part : tool.body_part_models) {
        if (appearance->model.size() != 1 || appearance->model[0] != part.race) {
            continue;
        }

        if (prev != part.part) {
            prev = part.part;
            model_numbers.clear();
        }

        if (creature_->appearance.phenotype != part.phenotype
            && phenotype->fallback != part.phenotype) {
            continue;
        } else if (part.female && creature_->gender != 1) {
            continue;
        }

        if (model_numbers.contains(part.number)) { continue; }
        model_numbers.insert(part.number);

        if ("bicepl" == part.part) {
            auto was = ui->bicep_left->blockSignals(true);
            ui->bicep_left->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.bicep_left) {
                ui->bicep_left->setCurrentIndex(ui->bicep_left->count() - 1);
            }
            ui->bicep_left->blockSignals(was);
        } else if ("bicepr" == part.part) {
            auto was = ui->bicep_right->blockSignals(true);
            ui->bicep_right->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.bicep_right) {
                ui->bicep_right->setCurrentIndex(ui->bicep_right->count() - 1);
            }
            ui->bicep_right->blockSignals(was);
        } else if ("chest" == part.part) {
            auto was = ui->torso->blockSignals(true);
            ui->torso->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.torso) {
                ui->torso->setCurrentIndex(ui->torso->count() - 1);
            }
            ui->torso->blockSignals(was);
        } else if ("footl" == part.part) {
            auto was = ui->foot_left->blockSignals(true);
            ui->foot_left->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.foot_left) {
                ui->foot_left->setCurrentIndex(ui->foot_left->count() - 1);
            }
            ui->foot_left->blockSignals(was);
        } else if ("footr" == part.part) {
            auto was = ui->foot_right->blockSignals(true);
            ui->foot_right->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.foot_right) {
                ui->foot_right->setCurrentIndex(ui->foot_right->count() - 1);
            }
            ui->foot_right->blockSignals(was);
        } else if ("forel" == part.part) {
            auto was = ui->forearm_left->blockSignals(true);
            ui->forearm_left->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.forearm_left) {
                ui->forearm_left->setCurrentIndex(ui->forearm_left->count() - 1);
            }
            ui->forearm_left->blockSignals(was);
        } else if ("forer" == part.part) {
            auto was = ui->forearm_right->blockSignals(true);
            ui->forearm_right->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.forearm_right) {
                ui->forearm_right->setCurrentIndex(ui->forearm_right->count() - 1);
            }
            ui->forearm_right->blockSignals(was);
        } else if ("handl" == part.part) {
            auto was = ui->hand_left->blockSignals(true);
            ui->hand_left->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.hand_left) {
                ui->hand_left->setCurrentIndex(ui->hand_left->count() - 1);
            }
            ui->hand_left->blockSignals(was);
        } else if ("handr" == part.part) {
            auto was = ui->hand_right->blockSignals(true);
            ui->hand_right->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.hand_right) {
                ui->hand_right->setCurrentIndex(ui->hand_right->count() - 1);
            }
            ui->hand_right->blockSignals(was);
        } else if ("head" == part.part) {
            auto was = ui->head->blockSignals(true);
            ui->head->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.head) {
                ui->head->setCurrentIndex(ui->head->count() - 1);
            }
            ui->head->blockSignals(was);
        } else if ("legl" == part.part) {
            auto was = ui->thigh_left->blockSignals(true);
            ui->thigh_left->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.thigh_left) {
                ui->thigh_left->setCurrentIndex(ui->thigh_left->count() - 1);
            }
            ui->thigh_left->blockSignals(was);
        } else if ("legr" == part.part) {
            auto was = ui->thigh_right->blockSignals(true);
            ui->thigh_right->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.thigh_right) {
                ui->thigh_right->setCurrentIndex(ui->thigh_right->count() - 1);
            }
            ui->thigh_right->blockSignals(was);
        } else if ("neck" == part.part) {
            auto was = ui->neck->blockSignals(true);
            ui->neck->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.neck) {
                ui->neck->setCurrentIndex(ui->neck->count() - 1);
            }
            ui->neck->blockSignals(was);
        } else if ("pelvis" == part.part) {
            auto was = ui->pelvis->blockSignals(true);
            ui->pelvis->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.pelvis) {
                ui->pelvis->setCurrentIndex(ui->pelvis->count() - 1);
            }
            ui->pelvis->blockSignals(was);
        } else if ("shinl" == part.part) {
            auto was = ui->shin_left->blockSignals(true);
            ui->shin_left->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.shin_left) {
                ui->shin_left->setCurrentIndex(ui->shin_left->count() - 1);
            }
            ui->shin_left->blockSignals(was);
        } else if ("shinr" == part.part) {
            auto was = ui->shin_right->blockSignals(true);
            ui->shin_right->addItem(QString::number(part.number), part.number);
            if (part.number == creature_->appearance.body_parts.shin_right) {
                ui->shin_right->setCurrentIndex(ui->shin_right->count() - 1);
            }
            ui->shin_right->blockSignals(was);
        }
    }
}

QPixmap CreatureAppearanceView::getPixmapIcon(nw::CreatureColors::type color) const
{
    auto selected = creature_->appearance.colors[color];

    int col = selected % 16;
    int row = selected / 16;
    QRect rect(col * 32, row * 32, 32, 32);

    // Extract the region of the image corresponding to the selected cell
    if (color == nw::CreatureColors::hair) {
        return mvpal_hair.copy(rect).scaled(24, 24);
    } else {
        return mvpal_skin.copy(rect).scaled(24, 24);
    }
}

// == public slots ============================================================
// ============================================================================

void CreatureAppearanceView::onAppearanceChange(int index)
{
    Q_UNUSED(index);
    if (!creature_) { return; }
    creature_->appearance.id = static_cast<uint16_t>(ui->appearance->currentData(Qt::UserRole).toInt());
    auto appearance = nw::kernel::rules().appearances.get(nw::Appearance::make(creature_->appearance.id));
    if (appearance) {
        is_dynamic_ = appearance->model.size() == 1;
    }
    updateEnabled();
    emit dataChanged();
}

void CreatureAppearanceView::onBodyPartChanged(int index)
{
    Q_UNUSED(index);
    auto objname = sender()->objectName();
    QComboBox* cb = static_cast<QComboBox*>(sender());
    if (objname == "bicep_left") {
        creature_->appearance.body_parts.bicep_left = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "bicep_right") {
        creature_->appearance.body_parts.bicep_right = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "torso") {
        creature_->appearance.body_parts.torso = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "foot_left") {
        creature_->appearance.body_parts.foot_left = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "foot_right") {
        creature_->appearance.body_parts.foot_right = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "forearm_left") {
        creature_->appearance.body_parts.forearm_left = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "forearm_right") {
        creature_->appearance.body_parts.forearm_right = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "hand_left") {
        creature_->appearance.body_parts.hand_left = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "hand_right") {
        creature_->appearance.body_parts.hand_right = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "head") {
        creature_->appearance.body_parts.head = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "thigh_left") {
        creature_->appearance.body_parts.thigh_left = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "thigh_right") {
        creature_->appearance.body_parts.thigh_right = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "neck") {
        creature_->appearance.body_parts.neck = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "pelvis") {
        creature_->appearance.body_parts.pelvis = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "shin_left") {
        creature_->appearance.body_parts.shin_left = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    } else if (objname == "shin_right") {
        creature_->appearance.body_parts.shin_right = static_cast<uint16_t>(cb->currentData(Qt::UserRole).toInt());
    }
}

void CreatureAppearanceView::onColorChanged(int color, int value)
{
    creature_->appearance.colors[color] = uint8_t(value);
    ui->hair->setIcon(getPixmapIcon(nw::CreatureColors::hair));
    ui->skin->setIcon(getPixmapIcon(nw::CreatureColors::skin));
    ui->tatoo1->setIcon(getPixmapIcon(nw::CreatureColors::tatoo1));
    ui->tatoo2->setIcon(getPixmapIcon(nw::CreatureColors::tatoo2));
}

void CreatureAppearanceView::onOpenColorSelector()
{
    auto dialog = CreatureColorSelectionDialog(this);
    connect(dialog.selector(), &CreatureColorSelectorView::colorChange, this, &CreatureAppearanceView::onColorChanged);
    dialog.selector()->setColors(creature_->appearance.colors);
    if (sender()->objectName() == "hair") {
        dialog.selector()->setIndex(nw::CreatureColors::hair);
    } else if (sender()->objectName() == "skin") {
        dialog.selector()->setIndex(nw::CreatureColors::skin);
    } else if (sender()->objectName() == "tatoo1") {
        dialog.selector()->setIndex(nw::CreatureColors::tatoo1);
    } else if (sender()->objectName() == "tatoo2") {
        dialog.selector()->setIndex(nw::CreatureColors::tatoo2);
    }

    if (dialog.exec() == QDialog::Accepted) {
    }
}

void CreatureAppearanceView::onPhenotypeChanged(int index)
{
    Q_UNUSED(index);
    creature_->appearance.phenotype = ui->phenotype->currentData(Qt::UserRole).toInt();

    ui->bicep_left->blockSignals(true);
    LOG_F(INFO, "Item count before clear: {}", ui->bicep_left->count());
    ui->bicep_left->clear();
    LOG_F(INFO, "Item count after clear: {}", ui->bicep_left->count());
    ui->bicep_left->blockSignals(false);
    ui->bicep_right->blockSignals(true);
    ui->bicep_right->clear();
    ui->bicep_right->blockSignals(false);
    ui->torso->blockSignals(true);
    ui->torso->clear();
    ui->torso->blockSignals(false);
    ui->foot_left->blockSignals(true);
    ui->foot_left->clear();
    ui->foot_left->blockSignals(false);
    ui->foot_right->blockSignals(true);
    ui->foot_right->clear();
    ui->foot_right->blockSignals(false);
    ui->forearm_left->blockSignals(true);
    ui->forearm_left->clear();
    ui->forearm_left->blockSignals(false);
    ui->forearm_right->blockSignals(true);
    ui->forearm_right->clear();
    ui->forearm_right->blockSignals(false);
    ui->hand_left->blockSignals(true);
    ui->hand_left->clear();
    ui->hand_left->blockSignals(false);
    ui->hand_right->blockSignals(true);
    ui->hand_right->clear();
    ui->hand_right->blockSignals(false);
    ui->head->blockSignals(true);
    ui->head->clear();
    ui->head->blockSignals(false);
    ui->thigh_left->blockSignals(true);
    ui->thigh_left->clear();
    ui->thigh_left->blockSignals(false);
    ui->thigh_right->blockSignals(true);
    ui->thigh_right->clear();
    ui->thigh_right->blockSignals(false);
    ui->neck->blockSignals(true);
    ui->neck->clear();
    ui->neck->blockSignals(false);
    ui->pelvis->blockSignals(true);
    ui->pelvis->clear();
    ui->pelvis->blockSignals(false);
    ui->shin_left->blockSignals(true);
    ui->shin_left->clear();
    ui->shin_left->blockSignals(false);
    ui->shin_right->blockSignals(true);
    ui->shin_right->clear();
    ui->shin_right->blockSignals(false);

    loadBodyParts();
}

// == private =================================================================
// ============================================================================

void CreatureAppearanceView::updateEnabled()
{
    ui->phenotype->setEnabled(is_dynamic_);
    if (!is_dynamic_) { ui->phenotype->setCurrentIndex(-1); }
    ui->hair->setEnabled(is_dynamic_);
    ui->skin->setEnabled(is_dynamic_);
    ui->tatoo1->setEnabled(is_dynamic_);
    ui->tatoo2->setEnabled(is_dynamic_);

    ui->bicep_left->setEnabled(is_dynamic_);
    ui->bicep_right->setEnabled(is_dynamic_);
    ui->torso->setEnabled(is_dynamic_);
    ui->foot_left->setEnabled(is_dynamic_);
    ui->foot_right->setEnabled(is_dynamic_);
    ui->forearm_left->setEnabled(is_dynamic_);
    ui->forearm_right->setEnabled(is_dynamic_);
    ui->hand_left->setEnabled(is_dynamic_);
    ui->hand_right->setEnabled(is_dynamic_);
    ui->head->setEnabled(is_dynamic_);
    ui->thigh_left->setEnabled(is_dynamic_);
    ui->thigh_right->setEnabled(is_dynamic_);
    ui->neck->setEnabled(is_dynamic_);
    ui->pelvis->setEnabled(is_dynamic_);
    ui->shin_left->setEnabled(is_dynamic_);
    ui->shin_right->setEnabled(is_dynamic_);
}
