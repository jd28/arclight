#include "creatureappearanceview.h"
#include "ui_creatureappearanceview.h"

#include "../ColorSelectorDialog/creaturecolorselectordialog.h"
#include "../ColorSelectorDialog/creaturecolorselectorview.h"
#include "../util/strings.h"

#include "nw/kernel/Rules.hpp"
#include "nw/objects/Creature.hpp"

#include <QAbstractItemView>

CreatureAppearanceView::CreatureAppearanceView(nw::Creature* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::CreatureAppearanceView)
{
    ui->setupUi(this);
    mvpal_hair = QPixmap(":/resources/images/mvpal_hair.png");
    mvpal_skin = QPixmap(":/resources/images/mvpal_skin.png");

    connect(ui->hair, &QPushButton::clicked, this, &CreatureAppearanceView::onOpenColorSelector);
    connect(ui->skin, &QPushButton::clicked, this, &CreatureAppearanceView::onOpenColorSelector);
    connect(ui->tatoo1, &QPushButton::clicked, this, &CreatureAppearanceView::onOpenColorSelector);
    connect(ui->tatoo2, &QPushButton::clicked, this, &CreatureAppearanceView::onOpenColorSelector);
    connect(ui->parts, &CreaturePartsView::updateModel, this, &CreatureAppearanceView::updateModel);

    obj_ = obj;
    is_dynamic_ = nw::string::icmp(nw::kernel::rules().appearances.entries[*obj_->appearance.id].model_type, "P");

    ui->parts->setCreature(obj_);
    ui->parts->setUndoStack(undoStack());

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
    auto selected = obj_->appearance.colors[color];

    int col = selected % 16;
    int row = selected / 16;
    QRect rect(col * 32, row * 32, 32, 32);

    if (color == nw::CreatureColors::hair) {
        return mvpal_hair.copy(rect).scaled(24, 24);
    } else {
        return mvpal_skin.copy(rect).scaled(24, 24);
    }
}

// == public slots ============================================================
// ============================================================================

void CreatureAppearanceView::onColorChanged(int color, int value)
{
    Q_UNUSED(color);
    Q_UNUSED(value);

    ui->hair->setIcon(getPixmapIcon(nw::CreatureColors::hair));
    ui->skin->setIcon(getPixmapIcon(nw::CreatureColors::skin));
    ui->tatoo1->setIcon(getPixmapIcon(nw::CreatureColors::tatoo1));
    ui->tatoo2->setIcon(getPixmapIcon(nw::CreatureColors::tatoo2));
}

void CreatureAppearanceView::onOpenColorSelector()
{
    auto dialog = CreatureColorSelectionDialog(obj_, undoStack(), this);
    connect(dialog.selector(), &CreatureColorSelectorView::colorChange, this,
        &CreatureAppearanceView::onColorChanged);

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

// == private =================================================================
// ============================================================================

void CreatureAppearanceView::updateEnabled()
{
    ui->hair->setEnabled(is_dynamic_);
    ui->skin->setEnabled(is_dynamic_);
    ui->tatoo1->setEnabled(is_dynamic_);
    ui->tatoo2->setEnabled(is_dynamic_);
}
