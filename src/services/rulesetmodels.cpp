#include "rulesetmodels.h"

#include <nw/kernel/Resources.hpp>
#include <nw/resources/Resource.hpp>

#include <QIODevice>
#include <QMimeData>

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

// == SoundModel ==============================================================
// ============================================================================

SoundModel::SoundModel(const nw::Vector<nw::Resref>& sounds, QObject* parent)
    : QAbstractListModel(parent)
{
    for (const auto& it : sounds) {
        sounds_.push_back(to_qstring(it.view()));
    }
}

SoundModel::SoundModel(QObject* parent)
    : QAbstractListModel(parent)
{
    auto visitor = [this](const nw::Resource& res) {
        if (nw::ResourceType::check_category(nw::ResourceType::sound, res.type)) {
            sounds_.push_back(to_qstring(res.resref.view()));
        }
    };

    nw::kernel::resman().visit(visitor);
}

void SoundModel::addSound(const QString& sound)
{
    beginInsertRows(QModelIndex(), sounds_.size(), sounds_.size());
    sounds_.append(sound);
    endInsertRows();
}

void SoundModel::removeSound(int row)
{
    if (row < 0 || row >= sounds_.size()) { return; }
    beginRemoveRows(QModelIndex(), row, row);
    sounds_.removeAt(row);
    endRemoveRows();
}

int SoundModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant SoundModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) { return {}; }
    return sounds_[index.row()];
}

QModelIndex SoundModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    if (row < int(sounds_.size())) {
        return createIndex(row, column);
    }
    return {};
}

QModelIndex SoundModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child)
    return {};
}

int SoundModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) { return 0; }
    return static_cast<int>(sounds_.size());
}

Qt::ItemFlags SoundModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::ItemIsDropEnabled;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QStringList SoundModel::mimeTypes() const
{
    return {"application/x-sound-list"};
}

QMimeData* SoundModel::mimeData(const QModelIndexList& indexes) const
{
    auto* mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    stream << reinterpret_cast<uintptr_t>(this);
    for (const QModelIndex& index : indexes) {
        if (index.isValid()) {
            stream << sounds_[index.row()];
            stream << index.row();
        }
    }

    mimeData->setData("application/x-sound-list", encodedData);
    return mimeData;
}

bool SoundModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (row == -1) {
        row = parent.row();
    }

    if (action == Qt::IgnoreAction || !data->hasFormat("application/x-sound-list")) {
        return false;
    }

    QByteArray encodedData = data->data("application/x-sound-list");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    uintptr_t source_ptr;
    stream >> source_ptr;
    auto source = reinterpret_cast<const SoundModel*>(source_ptr);

    QVector<QPair<QString, int>> newSounds;
    QString sound;
    int source_row;
    while (!stream.atEnd()) {
        stream >> sound;
        stream >> source_row;
        newSounds.append({sound, source_row});
    }

    if (newSounds.isEmpty()) { return false; }

    int insertRow = (row != -1) ? row : sounds_.size();
    if (source == this) {
        for (const auto& [_, souce_row] : newSounds) {
            if (souce_row < insertRow) { --insertRow; }
            removeSound(souce_row);
        }
    }

    beginInsertRows(QModelIndex(), insertRow, insertRow + newSounds.size() - 1);
    for (const auto& [source_sound, _] : newSounds) {
        sounds_.insert(insertRow++, source_sound);
    }
    endInsertRows();

    return true;
}

Qt::DropActions SoundModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

SoundSortFilterProxyModel::SoundSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

void SoundSortFilterProxyModel::setFilter(const QString& filter)
{
    filter_ = filter;
    invalidateFilter();
}

bool SoundSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (filter_.isEmpty()) { return true; }

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QString value = sourceModel()->data(index, Qt::DisplayRole).toString();
    return value.contains(filter_, Qt::CaseInsensitive);
}
