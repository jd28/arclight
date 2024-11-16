#include "colorselectordialog.h"

#include "colorselectorview.h"

#include <QVBoxLayout>

ColorSelectionDialog::ColorSelectionDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Select Character Colors");

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Load image and setup color selector
    selector_ = new ColorSelectorView(this);

    layout->addWidget(selector_);

    // Add controls
    // QComboBox* comboBox = new QComboBox();
    // comboBox->addItems({"Tattoo 1", "Tattoo 2"}); // Add items as needed
    // QPushButton* okButton = new QPushButton("OK");
    // QPushButton* cancelButton = new QPushButton("Cancel");

    // QHBoxLayout* controlsLayout = new QHBoxLayout();
    // controlsLayout->addWidget(comboBox);
    // controlsLayout->addWidget(okButton);
    // controlsLayout->addWidget(cancelButton);
    // layout->addLayout(controlsLayout);

    // connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    // connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}
