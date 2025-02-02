#include "rulesetmodels.h"

RuleFilterProxyModel::RuleFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0);
}

bool RuleFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    auto* model = sourceModel();
    QModelIndex idx = model->index(source_row, 0, source_parent);
    const auto is_valid = model->data(idx, Qt::UserRole + 2);
    return is_valid.toBool();
}

int mapProxyRowToSourceRow(const QSortFilterProxyModel* model, int row)
{
    QModelIndex pi = model->index(row, 0);
    if (!pi.isValid()) { return -1; }

    QModelIndex si = model->mapToSource(pi);
    if (!si.isValid()) { return -1; }

    return si.row();
}

int mapSourceRowToProxyRow(const QAbstractItemModel* source,
    const QSortFilterProxyModel* proxy, int row)
{
    QModelIndex si = source->index(row, 0);
    if (!si.isValid()) { return -1; }

    QModelIndex pi = proxy->mapFromSource(si);
    if (!pi.isValid()) { return -1; }

    return pi.row();
}
