#include "itemmodels.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

auto findStandardItemIndex(QStandardItemModel* model, int value) -> int
{
    for (int i = 0; i < model->rowCount(); ++i) {
        if (model->item(i)->data().toInt() == value) {
            return i;
        }
    }
    return -1;
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
