#include "soundview.h"
#include "ui_soundview.h"

SoundView::SoundView(QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::SoundView)
{
    ui->setupUi(this);
}

SoundView::~SoundView()
{
    delete ui;
}
