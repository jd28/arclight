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
