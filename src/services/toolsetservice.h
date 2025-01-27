#ifndef TOOLSETSERVICE_H
#define TOOLSETSERVICE_H

#include "util/strings.h"

#include "nw/config.hpp"
#include "nw/kernel/Kernel.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"

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
            return {};

        case Qt::DisplayRole:
            if (info.valid()) {
                return to_qstring(nw::kernel::strings().get(info.name));
            }

        case Qt::UserRole:
            return info.valid();
        }
    }

private:
    nw::PVector<T>* entries_;
};

class RuleFilterProxyModel : public QSortFilterProxyModel {
public:
    explicit RuleFilterProxyModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
        sort(0);
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        auto* model = sourceModel();
        QModelIndex idx = model->index(source_row, 0, source_parent);
        const auto is_valid = model->data(idx, Qt::UserRole);
        return is_valid.toBool();
    }
};

struct PartModels {
    nw::String part;
    char race = 'h';
    int phenotype = 0;
    bool female = false;
    int number = 0;

    auto operator<=>(const PartModels&) const noexcept = default;
    bool operator==(const PartModels&) const noexcept = default;
};

struct ToolsetService : public nw::kernel::Service {
    const static std::type_index type_index;

    ToolsetService(nw::MemoryResource* memory);

    virtual ~ToolsetService() = default;

    virtual void initialize(nw::kernel::ServiceInitTime time) override;

    nw::Vector<PartModels> body_part_models;
    RuleTypeModel<nw::BaseItemInfo>* base_item_model = nullptr;
    RuleTypeModel<nw::ClassInfo>* class_model = nullptr;
};

inline ToolsetService& toolset()
{
    auto res = nw::kernel::services().get_mut<ToolsetService>();
    if (!res) {
        throw std::runtime_error("kernel: unable to load toolset service");
    }
    return *res;
}

#endif // TOOLSETSERVICE_H
