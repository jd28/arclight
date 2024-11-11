#include "strreftextedit.h"
#include "ui_strreftextedit.h"

#include "util/strings.h"

#include "nw/kernel/Strings.hpp"

StrrefTextEdit::StrrefTextEdit(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StrrefTextEdit)
{
    ui->setupUi(this);

    ui->strref->setMaximum(0x0FFFFFFF); // [TODO] make sure this is right.
    ui->strrefShow->setChecked(locstring_.strref() != 0xFFFFFFFF);
    ui->feminine->setDisabled(lang_ == nw::LanguageID::english);
    ui->language->addItem("English", static_cast<int>(nw::LanguageID::english));
    ui->language->addItem("French", static_cast<int>(nw::LanguageID::french));
    ui->language->addItem("German", static_cast<int>(nw::LanguageID::german));
    ui->language->addItem("Italian", static_cast<int>(nw::LanguageID::italian));
    ui->language->addItem("Spanish", static_cast<int>(nw::LanguageID::spanish));
    ui->language->addItem("Polish", static_cast<int>(nw::LanguageID::polish));

    connect(ui->language, &QComboBox::currentIndexChanged, this, &StrrefTextEdit::onLanguageChanged);
    connect(ui->strref, &QSpinBox::valueChanged, this, &StrrefTextEdit::onStrrefChanged);
    connect(ui->strrefShow, &QCheckBox::toggled, this, &StrrefTextEdit::onStrrefShowToggled);
    connect(ui->feminine, &QCheckBox::toggled, this, &StrrefTextEdit::onFeminineToggled);
    connect(ui->textEdit, &QTextEdit::textChanged, this, &StrrefTextEdit::onTextEditChanged);
}

StrrefTextEdit::~StrrefTextEdit()
{
    delete ui;
}

const nw::LocString& StrrefTextEdit::locstring() const
{
    return locstring_;
}

void StrrefTextEdit::setLocstring(const nw::LocString& newLocstring)
{
    locstring_ = newLocstring;
    if (locstring_.strref() != 0xFFFFFFFF) {
        ui->strref->setValue(int(locstring_.strref()));
        ui->strrefShow->setChecked(true);
        updateTextEdit();
    } else {
        int i = 0;
        for (auto lang : {nw::LanguageID::english,
                 nw::LanguageID::french,
                 nw::LanguageID::german,
                 nw::LanguageID::italian,
                 nw::LanguageID::spanish,
                 nw::LanguageID::polish}) {
            if (locstring_.contains(lang, ui->feminine->isChecked())) {
                ui->language->setCurrentIndex(i);
            }
            ++i;
        }
    }
}

void StrrefTextEdit::updateTextEdit()
{
    ui->textEdit->blockSignals(true);
    if (ui->strrefShow->isChecked()) {
        auto string = nw::kernel::strings().get(locstring_.strref(), ui->feminine->isChecked());
        ui->textEdit->setDisabled(true);
        ui->textEdit->setText(to_qstring(string));
    } else {
        auto string = locstring_.get(lang_, ui->feminine->isChecked());
        ui->textEdit->setEnabled(true);
        ui->textEdit->setText(to_qstring(string));
        ui->textEdit->setDisabled(false);
    }
    ui->textEdit->blockSignals(false);
}

void StrrefTextEdit::onFeminineToggled(bool value)
{
    Q_UNUSED(value);
    updateTextEdit();
}

void StrrefTextEdit::onLanguageChanged(int index)
{
    if (index == -1) { return; }

    ui->strrefShow->blockSignals(true);
    ui->strrefShow->setChecked(false);
    ui->strrefShow->blockSignals(false);
    int lang = ui->language->itemData(index).toInt();
    lang_ = static_cast<nw::LanguageID>(lang);
    ui->feminine->setDisabled(lang_ == nw::LanguageID::english);
    updateTextEdit();
}

void StrrefTextEdit::onStrrefChanged(int value)
{
    locstring_.set_strref(static_cast<uint32_t>(value));
    ui->strrefShow->setChecked(true);
    updateTextEdit();
}

void StrrefTextEdit::onStrrefShowToggled(bool value)
{
    Q_UNUSED(value);
    updateTextEdit();
    ui->language->setDisabled(value);
}

void StrrefTextEdit::onTextEditChanged()
{
    locstring_.add(lang_, ui->textEdit->toPlainText().toStdString(), ui->feminine->isChecked());
}
