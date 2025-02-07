
#include "itemview.h"
#include "ui_itemview.h"

#include "../InventoryView/inventoryview.h"
#include "../VariableTableView/variabletableview.h"
#include "../strreftextedit.h"
#include "../util/strings.h"
#include "itemgeneralview.h"
#include "itemproperties.h"

#include "nw/kernel/Rules.hpp"
#include "nw/objects/Item.hpp"

#include <QScreen>
#include <QTextEdit>

static QRegularExpression resref_regex("^[a-z_]{0,16}$");

// == ItemView ================================================================
// ============================================================================

ItemView::ItemView(nw::Item* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::ItemView)
    , obj_{obj}
{
    ui->setupUi(this);

    auto bi_info = nw::kernel::rules().baseitems.get(obj_->baseitem);
    if (!bi_info || !bi_info->valid()) { return; }

    auto width = qApp->primaryScreen()->geometry().width();
    ui->splitter->setSizes(QList<int>() << width * 2 / 3 << width * 1 / 3);

    auto general = new ItemGeneralView(obj, this);
    general->setEnabled(!readOnly());
    ui->tabWidget->addTab(general, tr("General"));
    connect(general, &ItemGeneralView::baseItemChanged, this, &ItemView::onBaseItemChanged);

    auto properties = new ItemProperties(obj_, this);
    properties->setEnabled(!readOnly());
    ui->tabWidget->addTab(properties, tr("Properties"));

    inventory_ = new InventoryView(this);
    inventory_->setEnabled(!readOnly() && bi_info->is_container);
    inventory_->setObject(obj_);
    inventory_->setDragEnabled(false);
    ui->tabWidget->addTab(inventory_, tr("Inventory"));

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj_->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));

    auto description = new StrrefTextEdit(this);
    description->setEnabled(!readOnly());
    description->setLocstring(obj->description);
    ui->tabWidget->addTab(description, tr("Description"));

    auto comments = new QTextEdit(this);
    comments->setEnabled(!readOnly());
    comments->setProperty("last_text", to_qstring(obj_->common.comment));
    comments->setText(to_qstring(obj_->common.comment));
    ui->tabWidget->addTab(comments, tr("Comments"));

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ItemView::onTabChanged);
}

ItemView::~ItemView()
{
    delete ui;
}

void ItemView::onBaseItemChanged(nw::BaseItem bi)
{
    auto bi_info = nw::kernel::rules().baseitems.get(bi);
    if (!bi_info || !bi_info->valid()) { return; }
    inventory_->setEnabled(!readOnly() && bi_info->is_container);
}

void ItemView::onTabChanged(int index)
{
    if (index == 0 || index == 2) {
        ui->openGLWidget->show();
    } else {
        ui->openGLWidget->hide();
    }
}
