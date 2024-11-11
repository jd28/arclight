#include "creaturefeatselectormodel.h"

#include "../util/strings.h"

#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"

extern "C" {
#include "fzy/match.h"
}

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
        return sourceModel()->data(source_left, Qt::CheckStateRole).toBool() && !sourceModel()->data(source_right, Qt::CheckStateRole).toBool();
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
    return QVariant();
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
    if (!index.isValid())
        return QVariant();
    auto feat = nw::kernel::rules().feats.get(nw::Feat::make(index.row()));
    if (!feat || feat->name == 0xFFFFFFFF) { return QVariant(); }

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

    return QVariant();
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
    Q_UNUSED(role);
    if (!index.isValid() || index.column() != 2) {
        return false;
    }

    bool add = value.toBool();

    if (add) {
        creature_->stats.add_feat(nw::Feat::make(index.row()));
    } else {
        creature_->stats.remove_feat(nw::Feat::make(index.row()));
    }
    emit featsChanged();
    return true;
}
