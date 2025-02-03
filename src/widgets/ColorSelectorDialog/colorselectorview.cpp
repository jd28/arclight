#include "colorselectorview.h"
#include "ui_colorselectorview.h"

#include "nw/kernel/Strings.hpp"
#include "nw/objects/Item.hpp"
#include "nw/rules/items.hpp"

#include "util/objects.h"
#include "util/strings.h"

#include <QUndoCommand>

// == Undo Commands ===========================================================
// ============================================================================

class ColorValueCommand : public QUndoCommand {
public:
    ColorValueCommand(int part, int color, int previous, int newval,
        ColorSelectorView* view, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , part_{part}
        , color_{color}
        , previous_{previous}
        , newval_{newval}
        , view_{view}
    {
    }

    void undo() override
    {
        view_->setColor(part_, color_, previous_);
        emit view_->colorChange(part_, color_, previous_);
    }

    void redo() override
    {
        view_->setColor(part_, color_, newval_);
        emit view_->colorChange(part_, color_, newval_);
    }

private:
    int part_;
    int color_;
    int previous_;
    int newval_;
    ColorSelectorView* view_;
};

ColorSelectorView::ColorSelectorView(nw::Item* obj, bool has_parts, QUndoStack* undo, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ColorSelectorView)
    , obj_{obj}
    , undo_{undo}
    , has_parts_{has_parts}
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
    ui->part->addItem("Robe", int(nw::ItemModelParts::armor_robe));
    ui->part->addItem("Shin, Left", int(nw::ItemModelParts::armor_lshin));
    ui->part->addItem("Shin, Right", int(nw::ItemModelParts::armor_rshin));
    ui->part->addItem("Shoulder, Left", int(nw::ItemModelParts::armor_lshoul));
    ui->part->addItem("Shoulder, Right", int(nw::ItemModelParts::armor_rshoul));
    ui->part->addItem("Thigh, Left", int(nw::ItemModelParts::armor_lthigh));
    ui->part->addItem("Thigh, Right", int(nw::ItemModelParts::armor_rthigh));
    ui->part->addItem("Torso", int(nw::ItemModelParts::armor_torso));
    ui->part->setEnabled(has_parts_);

    ui->color->addItem("Cloth 1", int(nw::ItemColors::cloth1));
    ui->color->addItem("Cloth 2", int(nw::ItemColors::cloth2));
    ui->color->addItem("Leather 1", int(nw::ItemColors::leather1));
    ui->color->addItem("Leather 2", int(nw::ItemColors::leather2));
    ui->color->addItem("Metal 1", int(nw::ItemColors::metal1));
    ui->color->addItem("Metal 2", int(nw::ItemColors::metal2));

    connect(ui->clear, &QPushButton::clicked, this, &ColorSelectorView::onClearColorClicked);
    connect(ui->color, &QComboBox::currentIndexChanged, this, &ColorSelectorView::onColorChannelChanged);
    connect(ui->part, &QComboBox::currentIndexChanged, this, &ColorSelectorView::onPartChannelChanged);
    connect(ui->label, &ColorSelector::indexChanged, this, &ColorSelectorView::onValueChanged);
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

void ColorSelectorView::setColor(int part, int color, int value)
{
    if (part == -1) {
        ui->part->setCurrentIndex(0);
        onPartChannelChanged(0);
        obj_->model_colors[color] = uint8_t(value);
    } else {
        setPaletteIndex(static_cast<nw::ItemModelParts::type>(part));
        obj_->part_colors[part][color] = uint8_t(value);
    }

    setColorIndex(static_cast<nw::ItemColors::type>(color));
    ui->label->setIndex(value);
}

void ColorSelectorView::setColorIndex(nw::ItemColors::type index)
{
    for (int i = 0; i < ui->color->count(); ++i) {
        if (ui->color->itemData(i) == index) {
            ui->color->setCurrentIndex(i);
        }
    }
    onColorChannelChanged(index);
}

void ColorSelectorView::setHasParts(bool has_parts)
{
    has_parts_ = has_parts;
    ui->part->setEnabled(has_parts_);
    if (!has_parts_ && ui->part->currentIndex() != 0) {
        ui->part->setCurrentIndex(0);
    }
}

void ColorSelectorView::setPaletteIndex(nw::ItemModelParts::type index)
{
    for (int i = 0; i < ui->part->count(); ++i) {
        if (ui->part->itemData(i) == index) {
            ui->part->setCurrentIndex(i);
        }
    }
    onPartChannelChanged(index);
}

void ColorSelectorView::onClearColorClicked()
{
    int part = ui->part->currentData().toInt();
    onValueChanged(part == -1 ? 0 : -1);
}

void ColorSelectorView::onColorChannelChanged(int index)
{
    Q_UNUSED(index);

    auto color = ui->color->currentData().toInt();
    auto part = ui->part->currentData().toInt();

    switch (color) {
    default:
        break;
    case nw::ItemColors::metal1:
    case nw::ItemColors::metal2:
        ui->label->setPaletteImage(mvpal_metal);
        break;
    case nw::ItemColors::leather1:
    case nw::ItemColors::leather2:
        ui->label->setPaletteImage(mvpal_leather);
        break;
    case nw::ItemColors::cloth1:
    case nw::ItemColors::cloth2:
        ui->label->setPaletteImage(mvpal_cloth);
        break;
    }

    if (part == -1) {
        ui->label->setIndex(obj_->model_colors[color]);
    } else {
        ui->label->setIndex(obj_->part_colors[part][color]);
    }
}

void ColorSelectorView::onPartChannelChanged(int index)
{
    Q_UNUSED(index);
    auto color = ui->color->currentData().toInt();
    auto part = ui->part->currentData().toInt();
    if (part == -1) {
        ui->label->setIndex(obj_->model_colors[color]);
    } else {
        ui->label->setIndex(obj_->part_colors[part][color]);
    }
}

void ColorSelectorView::onValueChanged(int value)
{
    int part = ui->part->currentData().toInt();
    int color = ui->color->currentData().toInt();

    int old = 0;
    if (part == -1) {
        old = obj_->model_colors[color];
    } else {
        old = obj_->part_colors[part][color];
    }
    if (old == value) { return; }

    auto cmd = new ColorValueCommand(part, color, old, value, this);
    undo_->push(cmd);
}
