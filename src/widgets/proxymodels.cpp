#include "proxymodels.h"

extern "C" {
#include "fzy/match.h"
}

// == FuzzyProxyModel =========================================================
// ============================================================================

FuzzyProxyModel::FuzzyProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool FuzzyProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (filter_.isEmpty()) { return true; }
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    auto data = index.data(Qt::DisplayRole);
    return has_match(filter_.toStdString().c_str(), data.toString().toStdString().c_str());
}

void FuzzyProxyModel::onFilterChanged(QString filter)
{
    filter_ = std::move(filter);
    invalidateFilter();
}

// == EmptyFilterProxyModel ===================================================
// ============================================================================

EmptyFilterProxyModel::EmptyFilterProxyModel(int column, QObject* parent)
    : QSortFilterProxyModel(parent)
    , column_(column)
{
}

bool EmptyFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, column_, sourceParent);
    QVariant value = sourceModel()->data(index);

    if (!value.isValid()) { return false; }
    if (value.isNull()) { return false; }
    if (value.toString().isEmpty()) { return false; }

    return true;
}
