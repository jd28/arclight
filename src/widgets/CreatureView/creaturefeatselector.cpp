#include "creaturefeatselector.h"
#include "ui_creaturefeatselector.h"

#include "../checkboxdelegate.h"
#include "../util/strings.h"

#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"

extern "C" {
#include <fzy/match.h>
}

#include <QMouseEvent>
#include <QUndoCommand>

// == Undo Commands ===========================================================
// ============================================================================

class FeatChangeCommand : public QUndoCommand {
public:
    FeatChangeCommand(nw::Creature* creature, nw::Feat feat, bool add,
        CreatureFeatSelectorModel* model, const QModelIndex& index,
        QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , creature_(creature)
        , feat_(feat)
        , add_(add)
        , model_(model)
        , index_(index)
    {
        setText(add_ ? "Add Feat" : "Remove Feat");
    }

    void undo() override
    {
        if (add_) {
            creature_->stats.remove_feat(feat_);
        } else {
            creature_->stats.add_feat(feat_);
        }
        emit model_->dataChanged(index_, index_);
        emit model_->featsChanged();
    }

    void redo() override
    {
        if (add_) {
            creature_->stats.add_feat(feat_);
        } else {
            creature_->stats.remove_feat(feat_);
        }
        emit model_->dataChanged(index_, index_);
        emit model_->featsChanged();
    }

private:
    nw::Creature* creature_;
    nw::Feat feat_;
    bool add_;
    CreatureFeatSelectorModel* model_;
    QPersistentModelIndex index_;
};

// == CreatureFeatSelectorSortFilterProxy =====================================
// ============================================================================

CreatureFeatSelectorSortFilterProxy::CreatureFeatSelectorSortFilterProxy(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool CreatureFeatSelectorSortFilterProxy::filterAcceptsRow(int sourceRow,
    const QModelIndex& sourceParent) const
{
    auto feat = nw::kernel::rules().feats.get(nw::Feat::make(sourceRow));
    if (!feat || feat->name == 0xFFFFFFFF) { return false; }
    if (filter_empty_) { return true; }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    auto data = index.data(Qt::DisplayRole);
    auto needle = data.toString().toLower().toStdString();
    return has_match(filter_.c_str(), needle.c_str());
}

bool CreatureFeatSelectorSortFilterProxy::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    if (source_left.column() == 2) {
        bool left_checked = sourceModel()->data(source_left, Qt::DisplayRole).toBool();
        bool right_checked = sourceModel()->data(source_right, Qt::DisplayRole).toBool();
        return left_checked > right_checked;
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

void CreatureFeatSelectorSortFilterProxy::onFilterUpdated(const QString& filter)
{
    if (filter.size() == 0) {
        filter_ = filter.toStdString();
        filter_empty_ = true;
    } else {
        filter_ = filter.toLower().toStdString();
        filter_empty_ = false;
    }

    invalidateFilter();
}

// == CreatureFeatSelectorModel ==============================================
// ============================================================================

CreatureFeatSelectorModel::CreatureFeatSelectorModel(nw::Creature* creature, QObject* parent)
    : QAbstractTableModel(parent)
    , creature_{creature}
{
}

void CreatureFeatSelectorModel::setUndoStack(QUndoStack* undo)
{
    undo_ = undo;
}

QVariant CreatureFeatSelectorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        } else if (section == 1) {
            return tr("Type");
        } else if (section == 2) {
            return tr("Assigned");
        }
    }
    return {};
}

int CreatureFeatSelectorModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return int(nw::kernel::rules().feats.entries.size());
}

int CreatureFeatSelectorModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant CreatureFeatSelectorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }
    auto feat = nw::kernel::rules().feats.get(nw::Feat::make(index.row()));
    if (!feat || feat->name == 0xFFFFFFFF) { return {}; }

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return to_qstring(nw::kernel::strings().get(feat->name));
        } else if (index.column() == 1) {
            switch (feat->tools_categories) {
            default:
                return "Unknown";
            case 1:
                return "Combat Feat";
            case 2:
                return "Active Combat Feat";
            case 3:
                return "Defensive Feat";
            case 4:
                return "Magical Feat";
            case 5:
                return "Class / Racial Feat";
            case 6:
                return "Other Feat";
            }
        } else if (index.column() == 2) {
            return creature_->stats.has_feat(nw::Feat::make(index.row()));
        }
    }

    return {};
}

Qt::ItemFlags CreatureFeatSelectorModel::flags(const QModelIndex& index) const
{
    if (index.column() == 2) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
    return QAbstractTableModel::flags(index);
}

bool CreatureFeatSelectorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.column() != 2 || role != Qt::EditRole) {
        return false;
    }

    bool add = value.toBool();

    if (undo_) {
        undo_->push(new FeatChangeCommand(creature_, nw::Feat::make(index.row()), add, this, index));
    } else {
        if (add) {
            creature_->stats.add_feat(nw::Feat::make(index.row()));
        } else {
            creature_->stats.remove_feat(nw::Feat::make(index.row()));
        }
        emit dataChanged(index, index);
        emit featsChanged();
    }

    return true;
}

// == CreatureFeatSelector ====================================================
// ============================================================================

CreatureFeatSelector::CreatureFeatSelector(nw::Creature* creature, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::CreatureFeatSelector)
    , creature_{creature}
{
    ui->setupUi(this);
    model_ = new CreatureFeatSelectorModel(creature_, this);
    model_->setUndoStack(undoStack());
    filter_ = new CreatureFeatSelectorSortFilterProxy(this);
    filter_->setSourceModel(model_);
    ui->featTable->setModel(filter_);
    ui->featTable->model()->sort(0);
    ui->featTable->setItemDelegateForColumn(2, new CheckBoxDelegate(ui->featTable));
    ui->featTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->featTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->featTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->featTable->viewport()->installEventFilter(this);
    connect(ui->filterText, &QLineEdit::textChanged, filter_, &CreatureFeatSelectorSortFilterProxy::onFilterUpdated);
}

CreatureFeatSelector::~CreatureFeatSelector()
{
    delete ui;
}

bool CreatureFeatSelector::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = ui->featTable->indexAt(mouseEvent->pos());
        if (index.isValid() && index.column() == 2) {
            bool currentValue = index.data(Qt::DisplayRole).toBool();
            ui->featTable->model()->setData(index, !currentValue);
            return true;
        }
    }
    return ArclightTab::eventFilter(object, event);
}

CreatureFeatSelectorModel* CreatureFeatSelector::model() const
{
    return model_;
}

// == Slots ===================================================================
// ============================================================================
