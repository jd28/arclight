#include "creaturecolorselectorview.h"
#include "ui_creaturecolorselectorview.h"

#include "nw/objects/Appearance.hpp"

CreatureColorSelectorView::CreatureColorSelectorView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureColorSelector)
{
    ui->setupUi(this);
    colors_.fill(0);

    mvpal_hair = QPixmap(":/resources/images/mvpal_hair.png");
    mvpal_skin = QPixmap(":/resources/images/mvpal_skin.png");

    ui->color->addItem("Hair", int(nw::CreatureColors::hair));
    ui->color->addItem("Skin", int(nw::CreatureColors::skin));
    ui->color->addItem("Tatoo 1", int(nw::CreatureColors::tatoo1));
    ui->color->addItem("Tatoo 2", int(nw::CreatureColors::tatoo2));

    ui->label->setPaletteImage(mvpal_hair);
    ui->label->setIndex(colors_[nw::CreatureColors::hair]);

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

void CreatureColorSelectorView::setColors(std::array<uint8_t, 4> colors)
{
    colors_ = colors;
    auto index = ui->color->currentIndex();
    ui->label->setIndex(colors_[index]);
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
    ui->label->setIndex(colors_[index]);
}

void CreatureColorSelectorView::onValueChanged(int value)
{
    emit colorChange(ui->color->currentIndex(), value);
}
