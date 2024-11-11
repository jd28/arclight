#include "TlkModel.hpp"

#include <nw/log.hpp>

#include "../util/strings.h"

TlkModel::TlkModel(nw::Tlk* tlk, QObject* parent)
    : QAbstractTableModel(parent)
    , tlk_{tlk}
{
}

int TlkModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant TlkModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole) {
        return to_qstring(tlk_->get(static_cast<uint32_t>(index.row())));
    }

    return {};
}

int TlkModel::rowCount(const QModelIndex& /*parent*/) const
{
    return tlk_ ? static_cast<int>(tlk_->size()) : 0;
}
