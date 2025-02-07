#ifndef SERVICES_RULESETMODELS_H
#define SERVICES_RULESETMODELS_H

#include "nw/config.hpp"
#include "nw/log.hpp"

#include "util/strings.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

template <typename T>
struct RuleTypeModel : public QAbstractItemModel {
    explicit RuleTypeModel(nw::PVector<T>* entries, QObject* parent = nullptr)
        : QAbstractItemModel(parent)
        , entries_{entries}
    {
    }

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override
    {
        Q_UNUSED(column)
        Q_UNUSED(parent)
        if (row < int(entries_->size())) {
            return createIndex(row, column);
        }
        return {};
    }

    QModelIndex parent(const QModelIndex&) const override
    {
        return {};
    }

    int rowCount(const QModelIndex& parent) const override
    {
        Q_UNUSED(parent)
        return int(entries_->size());
    }

    int columnCount(const QModelIndex& parent) const override
    {
        Q_UNUSED(parent)
        return 1;
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() >= int(entries_->size())) {
            return {};
        }

        const auto& info = (*entries_)[index.row()];

        switch (role) {
        default:
            break;
        case Qt::DisplayRole:
            if (info.valid()) {
                return to_qstring(info.editor_name());
            }
            break;
        case Qt::UserRole + 1:
            return index.row();
        case Qt::UserRole + 2:
            return info.valid();
        }
        return {};
    }

private:
    nw::PVector<T>* entries_;
};

class RuleFilterProxyModel : public QSortFilterProxyModel {
public:
    explicit RuleFilterProxyModel(QObject* parent = nullptr);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

int mapProxyRowToSourceRow(const QSortFilterProxyModel* model, int row);
int mapSourceRowToProxyRow(const QAbstractItemModel* source,
    const QSortFilterProxyModel* proxy, int row);

#endif // SERVICES_RULESETMODELS_H
