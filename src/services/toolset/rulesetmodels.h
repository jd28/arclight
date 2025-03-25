#pragma once

#include <nw/config.hpp>
#include <nw/resources/Resref.hpp>

#include "../../widgets/util/strings.h"

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

// == SoundModel ==============================================================
// ============================================================================

// This really isn't totally general purpose it's designed around being in a sound
// object editor.. it's here because of the need for a global / all availabe sounds
// model.

class SoundModel : public QAbstractListModel {
public:
    explicit SoundModel(const nw::Vector<nw::Resref>& sounds, QObject* parent = nullptr);
    explicit SoundModel(QObject* parent = nullptr);

    void addSound(const QString& sound);
    void removeSound(int row);

    virtual int columnCount(const QModelIndex& parent) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    virtual QModelIndex parent(const QModelIndex& child) const override;
    virtual int rowCount(const QModelIndex& parent) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDropActions() const override;

    QVector<QString> sounds_;
};

class SoundSortFilterProxyModel : public QSortFilterProxyModel {
public:
    explicit SoundSortFilterProxyModel(QObject* parent = nullptr);
    void setFilter(const QString& filter);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QString filter_;
};
