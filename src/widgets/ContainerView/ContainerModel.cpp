#include "ContainerModel.hpp"

#include "../util/restypeicons.h"
#include "../util/strings.h"

extern "C" {
#include <fzy/match.h>
}

#include <nw/log.hpp>
#include <nw/resources/Erf.hpp>
#include <nw/resources/ResourceType.hpp>
#include <nw/util/platform.hpp>
#include <nw/util/string.hpp>

#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>

#include <algorithm>

namespace fs = std::filesystem;

// ----------------------------------------------------------------------------
// -- ContainerSortFilterProxyModel -------------------------------------------

ContainerSortFilterProxyModel::ContainerSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , filter_empty_{true}
{
}

void ContainerSortFilterProxyModel::onFilterUpdated(const QString& filter)
{
    if (filter.size() == 0) {
        filter_ = std::string{};
        filter_empty_ = true;
    } else {
        filter_ = filter.toLower().toStdString();
        filter_empty_ = false;
    }

    invalidateFilter();
}

bool ContainerSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (filter_empty_) {
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    auto data = index.data(Qt::DisplayRole);
    return has_match(filter_.c_str(), data.toString().toStdString().c_str());
}

// ----------------------------------------------------------------------------
// -- ContainerModel ----------------------------------------------------------

ContainerModel::ContainerModel(nw::Container* container, QObject* parent)
    : QAbstractTableModel(parent)
    , container_{container}
{
    resources_ = container_->all();
    std::sort(std::begin(resources_), std::end(resources_), [](const auto& a, const auto& b) {
        return a.name.filename() < b.name.filename();
    });
}

void ContainerModel::addFile(const nw::Resource& res, const fs::path& file)
{
    if (auto e = dynamic_cast<nw::Erf*>(container_)) {
        if (e->erase(res)) {
            e->add(file);
            auto it = std::find_if(std::begin(resources_), std::end(resources_), [&res](const auto& rd) {
                return rd.name == res;
            });
            *it = e->stat(res);
            int row = static_cast<int>(std::distance(std::begin(resources_), it));
            emit dataChanged(index(row, 0), index(row, columnCount()));
        } else {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            e->add(file);
            resources_.push_back(e->stat(res));
            endInsertRows();
        }
    }
}

void ContainerModel::addFile(const nw::Resource& res, const nw::ResourceData& data)
{
    if (auto e = dynamic_cast<nw::Erf*>(container_)) {
        if (e->erase(res)) {
            e->add(res, data.bytes);
            auto it = std::find_if(std::begin(resources_), std::end(resources_), [&res](const auto& rd) {
                return rd.name == res;
            });
            *it = e->stat(res);
            int row = static_cast<int>(std::distance(std::begin(resources_), it));
            emit dataChanged(index(row, 0), index(row, columnCount()));
        } else {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            e->add(res, data.bytes);
            resources_.push_back(e->stat(res));
            endInsertRows();
        }
    }
}

void ContainerModel::addFiles(const QStringList& files)
{
    bool yes_to_all = false;
    nw::Erf* erf = nullptr;

    if (!(erf = dynamic_cast<nw::Erf*>(container_))) {
        return;
    }

    for (const auto& f : files) {
        fs::path p{f.toStdString()};
        auto r = nw::Resource::from_path(p);
        if (!r.valid()) {
            // Put message eventually.
            continue;
        }
        if (nw::ResourceType::check_category(nw::ResourceType::container, r.type)) {
            mergeFiles({f});
        } else if (erf->contains(r)) {
            bool yes = false;
            if (!yes_to_all) {
                auto b = QMessageBox::question(nullptr, "Overwrite File",
                    to_qstring(fmt::format("'{}' already exists, would you like to overwrite?", r.filename())),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll);
                yes_to_all = b == QMessageBox::YesToAll;
                yes = b == QMessageBox::Yes;
            }
            if (yes_to_all || yes) {
                addFile(r, p);
            }
        } else {
            addFile(r, p);
        }
    }
}

void ContainerModel::mergeFiles(const QStringList& files)
{
    bool yes_to_all = false;
    nw::Erf* erf = nullptr;

    if (!(erf = dynamic_cast<nw::Erf*>(container_))) {
        return;
    }

    for (const auto& f : files) {
        fs::path p{f.toStdString()};
        auto r = nw::Resource::from_path(p);
        if (!r.valid()) {
            // Put message eventually.
            continue;
        }

        switch (r.type) {
        default:
            continue;
        case nw::ResourceType::erf:
        case nw::ResourceType::hak:
        case nw::ResourceType::mod: {
            nw::Erf e{p};
            for (const auto& rd : e.all()) {
                if (erf->contains(rd.name)) {
                    bool yes = false;
                    if (!yes_to_all) {
                        auto b = QMessageBox::question(nullptr, "Overwrite File",
                            to_qstring(fmt::format("'{}' already exists, would you like to overwrite?", rd.name.filename())),
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll);
                        yes_to_all = b == QMessageBox::YesToAll;
                        yes = b == QMessageBox::Yes;
                    }
                    if (yes_to_all || yes) {
                        addFile(rd.name, e.demand(rd.name));
                    }
                } else {
                    addFile(rd.name, e.demand(rd.name));
                }
            }
        } break;
        }
    }
}

void ContainerModel::setColumnCount(int cols)
{
    cols_ = cols;
}

bool ContainerModel::canDropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if (!mime->hasUrls()) {
        return false;
    }
    for (const auto& url : mime->urls()) {
        QFileInfo fn{url.fileName()};
        QFileInfo path{url.toLocalFile()};
        if (path.absolutePath().toStdString() == container_->working_directory()) {
            return false;
        }
        if (path.isDir()) {
            return false;
        }
        if (nw::ResourceType::from_extension(fn.completeSuffix().toStdString()) == nw::ResourceType::invalid) {
            return false;
        }
    }
    return true;
}

int ContainerModel::columnCount(const QModelIndex&) const
{
    return cols_;
}

QVariant ContainerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    size_t row = static_cast<size_t>(index.row());

    if (role == Qt::DecorationRole && index.column() == 0) {
        return restypeToIcon(resources_[row].name.type);
    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
        default:
            return {};
        case 0:
            return to_qstring(resources_[row].name.filename());
        case 1:
            return static_cast<int>(resources_[row].size);
        }
    }
    return {};
}

bool ContainerModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    QStringList files;
    for (const auto& url : data->urls()) {
        files.append(url.toLocalFile());
    }

    addFiles(files);

    return true;
}

Qt::ItemFlags ContainerModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}

QVariant ContainerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!container_) {
        return {};
    }
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return {};

    if (section == 0) {
        return "Resource";
    } else {
        return "Size (bytes)";
    }
}

QMimeData* ContainerModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* md = new QMimeData;
    QList<QUrl> urls;
    for (const auto& idx : indexes) {
        size_t row = static_cast<size_t>(idx.row());
        auto name = resources_[row].name.filename();
        container_->extract_by_glob(name, container_->working_directory());
        auto fn = nw::path_to_string(container_->working_directory() / name);
        auto url = QUrl::fromLocalFile(to_qstring(fn));
        if (!urls.contains(url)) {
            urls << url;
        }
    }

    md->setUrls(std::move(urls));
    return md;
}

QStringList ContainerModel::mimeTypes() const
{
    return QStringList() << "text/uri-list";
}

bool ContainerModel::removeRows(int row, int count, const QModelIndex& index)
{
    nw::Erf* erf = nullptr;
    if (!(erf = dynamic_cast<nw::Erf*>(container_))) {
        return false;
    }

    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        erf->erase(resources_[row].name);
        resources_.erase(std::begin(resources_) + row);
    }
    endRemoveRows();
    return true;
}

int ContainerModel::rowCount(const QModelIndex& parent) const
{
    return !parent.isValid() ? static_cast<int>(container_->size()) : 0;
}

Qt::DropActions ContainerModel::supportedDropActions() const
{
    return Qt::CopyAction;
}
