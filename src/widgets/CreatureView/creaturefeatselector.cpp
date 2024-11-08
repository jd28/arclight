#include "creaturefeatselector.h"
#include "ui_creaturefeatselector.h"

#include "../checkboxdelegate.h"
#include "creaturefeatselectormodel.h"

CreatureFeatSelector::CreatureFeatSelector(nw::Creature* creature, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureFeatSelector)
    , creature_{creature}
{
    ui->setupUi(this);
    model_ = new CreatureFeatSelectorModel(creature_, this);
    filter_ = new CreatureFeatSelectorSortFilterProxy(this);
    filter_->setSourceModel(model_);
    ui->featTable->setModel(filter_);
    ui->featTable->model()->sort(0);
    ui->featTable->setItemDelegateForColumn(2, new CheckBoxDelegate(ui->featTable));
    ui->featTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->featTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->featTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->featTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->filterText, &QLineEdit::textChanged, filter_, &CreatureFeatSelectorSortFilterProxy::onFilterUpdated);
}

CreatureFeatSelector::~CreatureFeatSelector()
{
    delete ui;
}

CreatureFeatSelectorModel* CreatureFeatSelector::model() const
{
    return model_;
}

// == Slots ===================================================================
// ============================================================================
