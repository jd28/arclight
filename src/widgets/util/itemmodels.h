#pragma once

class QAbstractItemModel;
class QSortFilterProxyModel;
class QStandardItemModel;

int findStandardItemIndex(QStandardItemModel* model, int value);
int mapProxyRowToSourceRow(const QSortFilterProxyModel* model, int row);
int mapSourceRowToProxyRow(const QAbstractItemModel* source,
    const QSortFilterProxyModel* proxy, int row);
