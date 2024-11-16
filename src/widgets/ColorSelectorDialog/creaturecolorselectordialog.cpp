#include "creaturecolorselectordialog.h"

#include "creaturecolorselectorview.h"

#include <QVBoxLayout>

CreatureColorSelectionDialog::CreatureColorSelectionDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Select Character Colors");

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Load image and setup color selector
    selector_ = new CreatureColorSelectorView(this);

    layout->addWidget(selector_);
}
