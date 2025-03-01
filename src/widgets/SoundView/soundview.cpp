#include "soundview.h"
#include "ui_soundview.h"

#include "../VariableTableView/variabletableview.h"
#include "../util/strings.h"
#include "soundgeneralview.h"

#include <nw/kernel/Objects.hpp>

#include <QTextEdit>

SoundView::SoundView(nw::Resource res, QWidget* parent)
    : SoundView(nw::kernel::objects().load<nw::Sound>(res.resref), parent)
{
    owned_ = true;
}

SoundView::SoundView(nw::Sound* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::SoundView)
    , obj_{obj}
{
    ui->setupUi(this);
    auto general = new SoundGeneralView(obj_, this);
    ui->tabWidget->addTab(general, "General");
    addTab(general);

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj_->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
    addTab(variables);

    auto comments = new QTextEdit(this);
    comments->setText(to_qstring(obj_->common.comment));
    ui->tabWidget->addTab(comments, "Comments");
}

SoundView::~SoundView()
{
    delete ui;
    if (obj_ && owned_) {
        nw::kernel::objects().destroy(obj_->handle());
    }
}
