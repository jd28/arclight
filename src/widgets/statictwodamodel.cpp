#include "statictwodamodel.h"

#include "util/strings.h"

#include "nw/formats/StaticTwoDA.hpp"
#include "nw/kernel/Strings.hpp"

StaticTwoDAModel::StaticTwoDAModel(const nw::StaticTwoDA* tda, int name_column, QObject* parent)
    : QAbstractItemModel(parent)
    , tda_{tda}
    , name_column_{name_column}
{
}

void StaticTwoDAModel::setColumns(QList<int> columns)
{
    columns_ = std::move(columns);
}

QModelIndex StaticTwoDAModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if (row < 0
        || row >= int(tda_->rows())
        || column < 0
        || column >= int(columns_.empty() ? tda_->columns() : columns_.size())) {
        return {};
    }
    return createIndex(row, column);
}

QModelIndex StaticTwoDAModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return {};
}

int StaticTwoDAModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return int(tda_->rows());
}

int StaticTwoDAModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return int(columns_.empty() ? tda_->columns() : columns_.size());
}

QVariant StaticTwoDAModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return QVariant(); }
    if (role != Qt::DisplayRole) { return QVariant(); }

    int column = columns_.empty() ? index.column() : columns_[index.column()];
    if (column == name_column_) {
        int string_id;
        if (tda_->get_to(index.row(), column, string_id)) {
            return to_qstring(nw::kernel::strings().get(uint32_t(string_id)));
        }
        return QVariant();
    }

    std::string value;
    if (tda_->get_to(index.row(), column, value)) {
        return to_qstring(value);
    }

    return QVariant();
}
