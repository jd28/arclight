#include "textboxdialog.h"

#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

TextBoxDialog::TextBoxDialog(const QString& title, const QString& summary, int width, int height, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(title);

    auto* text = new QTextEdit(this);
    text->setPlainText(summary);
    text->setReadOnly(true);

    auto* close = new QPushButton("Close", this);
    connect(close, &QPushButton::clicked, this, &QDialog::accept);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(text);
    layout->addWidget(close);

    setLayout(layout);
    resize(width, height);
}
