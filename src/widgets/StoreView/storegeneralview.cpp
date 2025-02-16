#include "storegeneralview.h"
#include "ui_storegeneralview.h"

#include "../util/strings.h"

#include "nw/kernel/Rules.hpp"
#include "nw/objects/Store.hpp"

#include <QStandardItemModel>

StoreGeneralView::StoreGeneralView(nw::Store* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::StoreGeneralView)
    , obj_{obj}
{
    ui->setupUi(this);
    ui->name->setLocString(obj_->common.name);
    ui->tag->setText(to_qstring(obj_->tag().view()));
    ui->resref->setText(to_qstring(obj_->common.resref.view()));

    nw::Vector<int32_t>* list = &obj_->inventory.will_not_buy;
    if (list->empty() && obj_->inventory.will_only_buy.size()) {
        list = &obj_->inventory.will_only_buy;
        ui->will_not_buy->setChecked(false);
        ui->will_only_buy->setChecked(true);
    }

    connect(ui->will_not_buy, &QRadioButton::clicked, this, &StoreGeneralView::onRestrictionTypeChanged);
    connect(ui->will_only_buy, &QRadioButton::clicked, this, &StoreGeneralView::onRestrictionTypeChanged);

    restrict_model_ = new QStandardItemModel(this);
    QStringList headers;
    headers << "Base Items";
    restrict_model_->setHorizontalHeaderLabels(headers);

    int i = 0;
    for (const auto& it : nw::kernel::rules().baseitems.entries) {
        if (!it.valid()) { continue; }
        auto item = new QStandardItem(to_qstring(it.editor_name()));
        item->setCheckable(true);
        auto find = std::find(std::begin(*list), std::end(*list), i);
        item->setCheckState(find != std::end(*list) ? Qt::Checked : Qt::Unchecked);
        item->setData(i);
        restrict_model_->appendRow(item);
        ++i;
    }

    restrict_model_->sort(0);
    ui->restrictions->setModel(restrict_model_);
    ui->restrictions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(restrict_model_, &QStandardItemModel::itemChanged, this, &StoreGeneralView::onItemChanged);

    loadProperties();
}

StoreGeneralView::~StoreGeneralView()
{
    delete ui;
}

void StoreGeneralView::onItemChanged(QStandardItem* item)
{
    nw::Vector<int32_t>* list = nullptr;
    if (ui->will_not_buy->isChecked()) {
        list = &obj_->inventory.will_not_buy;
    } else if (ui->will_only_buy->isChecked()) {
        list = &obj_->inventory.will_only_buy;
    }
    if (!list) { return; }

    int baseitem = item->data().toInt();
    if (item->checkState() == Qt::Checked) {
        if (std::find(list->begin(), list->end(), baseitem) == list->end()) {
            list->push_back(baseitem);
        }
    } else {
        auto it = std::find(list->begin(), list->end(), baseitem);
        if (it != list->end()) { list->erase(it); }
    }

    LOG_F(INFO, "Store buyable item size: {}", list->size());
}

void StoreGeneralView::onRestrictionTypeChanged(bool checked)
{
    Q_UNUSED(checked);
    // Just swap the underlying vectors on toggle, since one ALWAYS should be empty
    // and the other has the baseitems.
    std::swap(obj_->inventory.will_not_buy, obj_->inventory.will_only_buy);
}

void StoreGeneralView::loadProperties()
{
    Property* grp_pricing = ui->properties->makeGroup("Pricing");

    auto prop = ui->properties->makeIntegerProperty("Sell Mark Up", obj_->markup, grp_pricing);
    prop->int_config.min = 0;
    prop->int_config.max = 1000;
    prop->on_set = [this](const QVariant& value) {
        obj_->markup = value.toInt();
    };

    prop = ui->properties->makeIntegerProperty("Buy Mark Down", obj_->markdown, grp_pricing);
    prop->int_config.min = 0;
    prop->int_config.max = 1000;
    prop->on_set = [this](const QVariant& value) {
        obj_->markup = value.toInt();
    };

    prop = ui->properties->makeBoolProperty("Black Market", obj_->blackmarket, grp_pricing);
    prop->on_set = [this](const QVariant& value) {
        obj_->blackmarket = value.toBool();
    };

    prop = ui->properties->makeIntegerProperty("Black Market Mark Down", obj_->blackmarket_markdown, grp_pricing);
    prop->int_config.min = 0;
    prop->int_config.max = 1000;
    prop->on_set = [this](const QVariant& value) {
        obj_->blackmarket_markdown = value.toInt();
    };

    prop = ui->properties->makeIntegerProperty("Identify Price", obj_->identify_price, grp_pricing);
    prop->int_config.min = -1;
    prop->int_config.max = INT_MAX;
    prop->on_set = [this](const QVariant& value) {
        obj_->identify_price = value.toInt();
    };

    prop = ui->properties->makeIntegerProperty("Maximum Price", obj_->max_price, grp_pricing);
    prop->int_config.min = -1;
    prop->int_config.max = INT_MAX;
    prop->on_set = [this](const QVariant& value) {
        obj_->max_price = value.toInt();
    };

    ui->properties->addProperty(grp_pricing);

#define ADD_SCRIPT(name, resref, grp)                                                                    \
    do {                                                                                                 \
        auto p = ui->properties->makeStringProperty(name, to_qstring(obj_->scripts.resref.view()), grp); \
        p->on_set = [this](const QVariant& value) {                                                      \
            obj_->scripts.resref = nw::Resref{value.toString().toStdString()};                           \
        };                                                                                               \
    } while (0)

    Property* grp_scripts = ui->properties->makeGroup("Scripts");
    ADD_SCRIPT("On Opened", on_opened, grp_scripts);
    ADD_SCRIPT("On Closed", on_closed, grp_scripts);
    ui->properties->addProperty(grp_scripts);

#undef ADD_SCRIPT
}
