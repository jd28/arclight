#include "creaturecolorselectorview.h"
#include "ui_creaturecolorselectorview.h"

#include "nw/objects/Appearance.hpp"
#include "nw/objects/Creature.hpp"

#include <QUndoCommand>

// == Undo Commands ===========================================================
// ============================================================================

class CreatureColorValueCommand : public QUndoCommand {
public:
    CreatureColorValueCommand(int color, int previous, int newval,
        CreatureColorSelectorView* view, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , color_{color}
        , previous_{previous}
        , newval_{newval}
        , view_{view}
    {
    }

    void undo() override
    {
        view_->setColor(color_, previous_);
        emit view_->colorChange(color_, previous_);
    }

    void redo() override
    {
        view_->setColor(color_, newval_);
        emit view_->colorChange(color_, newval_);
    }

private:
    int color_;
    int previous_;
    int newval_;
    CreatureColorSelectorView* view_;
};

CreatureColorSelectorView::CreatureColorSelectorView(nw::Creature* obj, QUndoStack* undo, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureColorSelector)
    , obj_{obj}
    , undo_{undo}
{
    ui->setupUi(this);

    mvpal_hair = QPixmap(":/resources/images/mvpal_hair.png");
    mvpal_skin = QPixmap(":/resources/images/mvpal_skin.png");

    ui->color->addItem("Hair", int(nw::CreatureColors::hair));
    ui->color->addItem("Skin", int(nw::CreatureColors::skin));
    ui->color->addItem("Tatoo 1", int(nw::CreatureColors::tatoo1));
    ui->color->addItem("Tatoo 2", int(nw::CreatureColors::tatoo2));

    ui->label->setPaletteImage(mvpal_hair);
    ui->label->setIndex(obj_->appearance.colors[nw::CreatureColors::hair]);

    connect(ui->color, &QComboBox::currentIndexChanged, this, &CreatureColorSelectorView::onColorChanelChanged);
    connect(ui->label, &ColorSelector::indexChanged, this, &CreatureColorSelectorView::onValueChanged);
}

CreatureColorSelectorView::~CreatureColorSelectorView()
{
    delete ui;
}

void CreatureColorSelectorView::setIndex(nw::CreatureColors::type index)
{
    ui->color->setCurrentIndex(index);
    onColorChanelChanged(index);
}

void CreatureColorSelectorView::setColor(int color, int value)
{
    setIndex(static_cast<nw::CreatureColors::type>(color));
    obj_->appearance.colors[color] = uint8_t(value);
    ui->label->setIndex(value);
}

void CreatureColorSelectorView::onColorChanelChanged(int index)
{
    switch (index) {
    case nw::CreatureColors::hair:
        ui->label->setPaletteImage(mvpal_hair);
        break;
    case nw::CreatureColors::skin:
    case nw::CreatureColors::tatoo1:
    case nw::CreatureColors::tatoo2:
        ui->label->setPaletteImage(mvpal_skin);
        break;
    }
    ui->label->setIndex(obj_->appearance.colors[index]);
}

void CreatureColorSelectorView::onValueChanged(int value)
{
    int color = ui->color->currentData().toInt();
    int old = obj_->appearance.colors[color];
    if (old == value) { return; }

    auto cmd = new CreatureColorValueCommand(color, old, value, this);
    undo_->push(cmd);
}
