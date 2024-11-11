#include "creatureappearanceview.h"
#include "ui_creatureappearanceview.h"

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

    int idx = 0;
    bool is_dynamic = false;
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
            LOG_F(INFO, "setting appearance: {}", idx);
            ui->appearance->setCurrentIndex(idx);
            is_dynamic = appearances.entries[i].model.size() <= 1;
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

    if (is_dynamic) {
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

    creature_ = creature;
}

CreatureAppearanceView::~CreatureAppearanceView()
{
    delete ui;
}

// == public slots ============================================================
// ============================================================================

void CreatureAppearanceView::onAppearanceChange(int index)
{
    Q_UNUSED(index);
    if (!creature_) { return; }
    creature_->appearance.id = static_cast<uint16_t>(ui->appearance->currentData(Qt::UserRole).toInt());
    emit dataChanged();
}
