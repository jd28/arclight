#include "itemsimplemodelselector.h"
#include "ui_itemsimplemodelselector.h"

#include "../../services/toolsetservice.h"

#include "nw/kernel/Rules.hpp"

#include <QStandardItemModel>

ItemSimpleModelSelector::ItemSimpleModelSelector(nw::BaseItem type, int current, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ItemSimpleModelSelector)
    , type_{type}
    , current_{current}
{
    ui->setupUi(this);
    auto bi_info = nw::kernel::rules().baseitems.get(type_);
    if (!bi_info || !bi_info->valid()) { return; }

    auto needle = fmt::format("i{}", bi_info->item_class.view());
    auto model = toolset().get_simple_models(needle);
    ui->listView->setModel(model);

    QModelIndex index = model->index(current_, 0);
    ui->listView->setCurrentIndex(index);
    ui->listView->scrollTo(index, QAbstractItemView::PositionAtCenter);
}

ItemSimpleModelSelector::~ItemSimpleModelSelector()
{
    delete ui;
}

QVariant ItemSimpleModelSelector::value() const
{
    QModelIndex currentIndex = ui->listView->currentIndex();
    if (!currentIndex.isValid()) { return -1; }
    return currentIndex.data(Qt::UserRole + 1);
}

int ItemSimpleModelSelector::currentIndex() const
{
    QModelIndex currentIndex = ui->listView->currentIndex();
    if (!currentIndex.isValid()) { return -1; }
    return currentIndex.row();
}

QString ItemSimpleModelSelector::currentText() const
{
    QModelIndex currentIndex = ui->listView->currentIndex();
    if (!currentIndex.isValid()) { return ""; }
    return currentIndex.data().toString();
}

void ItemSimpleModelSelector::setIconSize(QSize size)
{
    ui->listView->setIconSize(size);
    ui->listView->setGridSize(size + QSize(32, 32));
}
