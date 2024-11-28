#include "creatureappearanceview.h"
#include "ui_creatureappearanceview.h"

#include "../ColorSelectorDialog/creaturecolorselectordialog.h"
#include "../ColorSelectorDialog/creaturecolorselectorview.h"
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

    ui->parts->setCreature(creature_);

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

    for (int i = 0; i < ui->phenotype->count(); ++i) {
        if (ui->phenotype->itemData(i) == creature_->appearance.phenotype) {
            ui->phenotype->setCurrentIndex(i);
            break;
        }
    }

    ui->parts->clear();
    ui->parts->loadProperties();

    updateEnabled();
    emit dataChanged();
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
    ui->parts->clear();
    ui->parts->loadProperties();
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
    ui->parts->setEnabled(is_dynamic_);
}
