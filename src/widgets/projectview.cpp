#include "projectview.h"

extern "C" {
#include "fzy/match.h"
}
#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"
#include "nw/kernel/Objects.hpp"
#include "util/restypeicons.h"
#include "util/strings.h"

#include "nw/util/scope_exit.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMimeData>

inline QString read_object_name(const QString& path)
{
    QFileInfo fi(path);
    auto basename = fi.fileName();

    auto res_ = nw::Resource::from_filename(basename.toStdString());
    if (res_.type == nw::ResourceType::uti) {
        auto name = nw::Item::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    } else if (res_.type == nw::ResourceType::utc) {
        auto name = nw::Creature::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    } else if (res_.type == nw::ResourceType::are) {
        auto name = nw::Area::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    } else if (res_.type == nw::ResourceType::utd) {
        auto name = nw::Door::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    } else if (res_.type == nw::ResourceType::ute) {
        auto name = nw::Encounter::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    } else if (res_.type == nw::ResourceType::utm) {
        auto name = nw::Store::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    } else if (res_.type == nw::ResourceType::utp) {
        auto name = nw::Placeable::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    } else if (res_.type == nw::ResourceType::uts) {
        auto name = nw::Sound::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    } else if (res_.type == nw::ResourceType::utt) {
        auto name = nw::Trigger::get_name_from_file(path.toStdString());
        if (!name.empty()) {
            return to_qstring(name);
        }
    }

    return basename;
}

// == ProjectItem =============================================================
// ============================================================================

ProjectItem::ProjectItem(const QString& name, const QString& path, nw::StaticDirectory* module, ProjectItem* parent)
    : AbstractTreeItem(0, parent)
    , module_{module}
    , path_{path}
    , name_{name}
    , is_folder_{true}
{
}

ProjectItem::ProjectItem(const QString& name, const QString& path, nw::Resource res, nw::StaticDirectory* module, ProjectItem* parent)
    : AbstractTreeItem(0, parent)
    , module_{module}
    , path_{path}
    , name_{name}
    , res_{res}
    , is_folder_{false}
{
}

inline bool comparePaths(const QString& path1, const QString& path2)
{
    QString normalizedPath1 = QDir::fromNativeSeparators(QDir::cleanPath(path1));
    QString normalizedPath2 = QDir::fromNativeSeparators(QDir::cleanPath(path2));
    return normalizedPath1 == normalizedPath2;
}

QVariant ProjectItem::data(int column, int role) const
{
    if (column != 0) { return {}; }
    if (role == Qt::DisplayRole) {
        return name_;
    } else if (role == Qt::DecorationRole) {
        if (!is_folder_) {
            auto path = module_->get_canonical_path(res_);
            if (!comparePaths(to_qstring(path), path_)) {
                return ZFontIcon::icon(Fa6::FAMILY, Fa6::SOLID, Fa6::fa_circle_exclamation, Qt::red);
            }
            return restypeToIcon(res_.type);
        } else {
            auto color = QColor(42, 130, 218);
            return ZFontIcon::icon(Fa6::FAMILY, Fa6::SOLID, Fa6::fa_folder, color);
        }
    } else if (role == Qt::ToolTipRole) {
        if (!is_folder_) {
            auto path = to_qstring(module_->get_canonical_path(res_));
            if (!comparePaths(path, path_)) {
                return QString("%1 is shadowed by %2").arg(path_, path);
            } else {
                return to_qstring(res_.filename());
            }
        }
    }
    return {};
}

// == ProjectModel ============================================================
// ============================================================================

ProjectModel::ProjectModel(nw::StaticDirectory* module, QObject* parent)
    : AbstractTreeModel{parent}
    , module_{module}
    , path_{to_qstring(module_->path())}
{
    if (!setupDatabase()) {
        throw std::runtime_error("failed to open arclight meta database");
    }
}

int ProjectModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ProjectModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }
    auto item = reinterpret_cast<ProjectItem*>(index.internalPointer());
    return item->data(index.column(), role);
}

void ProjectModel::loadRootItems()
{
    walkDirectory(path_);
}

void ProjectModel::walkDirectory(const QString& path, ProjectItem* parent)
{
    QDir dir(path);
    watcher_.addPath(path);

    // Get the list of entries in the directory
    QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    foreach (const QFileInfo& fileInfo, list) {
        auto p = fileInfo.canonicalFilePath();

        ProjectItem* it = nullptr;
        nw::Resource res;
        if (fileInfo.isDir()) {
            it = new ProjectItem(fileInfo.baseName(), p, module_);
            walkDirectory(fileInfo.canonicalFilePath(), it);
        } else {
            res = nw::Resource::from_path(p.toStdString());
            if (res.valid()) {
                auto meta = getMetadata(p);
                bool insert = false;
                if (meta.lastModified.isValid()) {
                    qint64 lastModifiedOnDiskEpoch = fileInfo.lastModified().toMSecsSinceEpoch();
                    qint64 dbLastModifiedEpoch = meta.lastModified.toMSecsSinceEpoch();
                    insert = lastModifiedOnDiskEpoch > dbLastModifiedEpoch;
                } else {
                    insert = true;
                }
                if (insert) {
                    meta.object_name = read_object_name(p);
                    meta.size = fileInfo.size();
                    meta.lastModified = fileInfo.lastModified();
                    insertMetadata(p, meta);
                }

                it = new ProjectItem(meta.object_name, p, res, module_);
            }
        }

        if (it) {
            if (parent) {
                parent->appendChild(it);
            } else {
                addRootItem(it);
            }
        }
    }
}

bool ProjectModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    // Note parent parameter if dropping direcly on to a node, is that node, i.e. the new parent.
    // If dropping between nodes it's the parent of the new siblings.

    if (!data->hasFormat("application/x-arclight-projectitem")) {
        return false;
    }

    QByteArray data_ = data->data("application/x-arclight-projectitem");
    QDataStream stream(&data_, QIODevice::ReadOnly);
    qint64 senderPid;
    stream >> senderPid;
    if (senderPid != QCoreApplication::applicationPid()) {
        return false;
    }

    qlonglong ptr;
    stream >> ptr;
    const ProjectItem* node = reinterpret_cast<const ProjectItem*>(ptr);

    if (row != -1) { return false; }

    auto parent_node = reinterpret_cast<ProjectItem*>(parent.internalPointer());
    if (!parent_node) { return false; } // [TODO] Resasses this later

    if (parent_node->is_folder_) {
        return true;
    } else if (parent_node->parent_ == node->parent_) {
        return false;
    }

    auto result = AbstractTreeModel::canDropMimeData(data, action, row, column, parent);
    return result;
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    auto result = QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    return result;
}

ProjectItemMetadata ProjectModel::getMetadata(const QString& path)
{
    ProjectItemMetadata metadata;

    sqlite3_stmt* stmt = nullptr;
    SCOPE_EXIT([stmt] { sqlite3_finalize(stmt); });

    const char* sql = "SELECT object_name, size, last_modified FROM file_metadata WHERE path = ?";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        LOG_F(ERROR, "Failed to prepare statement: {}", sqlite3_errmsg(db_));
        return metadata;
    }

    if (sqlite3_bind_text(stmt, 1, path.toUtf8().constData(), -1, SQLITE_STATIC) != SQLITE_OK) {
        LOG_F(ERROR, "Failed to bind parameter: {}", sqlite3_errmsg(db_));
        return metadata;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        metadata.object_name = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        metadata.size = sqlite3_column_int64(stmt, 1);
        metadata.lastModified = QDateTime::fromString(QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))), Qt::ISODateWithMs);
    }

    return metadata;
}

void ProjectModel::insertMetadata(const QString& path, const ProjectItemMetadata& metadata)
{
    sqlite3_stmt* stmt = nullptr;
    SCOPE_EXIT([stmt] { sqlite3_finalize(stmt); });

    const char* sql = "REPLACE INTO file_metadata (path, object_name, size, last_modified) VALUES (?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        LOG_F(ERROR, "Failed to prepare statement: {}", sqlite3_errmsg(db_));
        return;
    }

    auto p = path.toStdString();
    auto n = metadata.object_name.toStdString();
    auto d = metadata.lastModified.toString(Qt::ISODateWithMs).toStdString();
    if (sqlite3_bind_text(stmt, 1, p.c_str(), static_cast<int>(p.size()), SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 2, n.c_str(), static_cast<int>(n.size()), SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_int64(stmt, 3, metadata.size) != SQLITE_OK
        || sqlite3_bind_text(stmt, 4, d.c_str(), static_cast<int>(d.size()), SQLITE_STATIC) != SQLITE_OK) {
        LOG_F(ERROR, "Failed to bind parameters:", sqlite3_errmsg(db_));
        return;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        LOG_F(ERROR, "Insert error: {}", sqlite3_errmsg(db_));
    }
}

QMimeData* ProjectModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    const ProjectItem* node = static_cast<ProjectItem*>(indexes[0].internalPointer());

    stream << QCoreApplication::applicationPid();
    stream << reinterpret_cast<qlonglong>(node);

    mimeData->setData("application/x-arclight-projectitem", data);
    return mimeData;
}

QStringList ProjectModel::mimeTypes() const
{
    return QStringList() << "application/x-arclight-projectitem";
}

bool ProjectModel::setupDatabase()
{
    QString dbPath = QDir(path_).filePath(".arclight_meta.db");

    // Open the database
    if (sqlite3_open(dbPath.toStdString().c_str(), &db_) != SQLITE_OK) {
        LOG_F(ERROR, "Cannot open database: {}", sqlite3_errmsg(db_));
        return false;
    }

    // Create table if it does not exist
    const char* sql = "CREATE TABLE IF NOT EXISTS file_metadata ("
                      "path TEXT PRIMARY KEY, "
                      "object_name TEXT, "
                      "size INTEGER, "
                      "last_modified DATETIME)";

    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        LOG_F(ERROR, "SQL error: {}", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db_);
        return false;
    }

    return true;
}

Qt::DropActions ProjectModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions ProjectModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

// == ProjectProxyModel =======================================================
// ============================================================================

ProjectProxyModel::ProjectProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool ProjectProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    auto item = static_cast<ProjectItem*>(index.internalPointer());
    if (item->res_.type == nw::ResourceType::git || item->res_.type == nw::ResourceType::gic) {
        return false;
    }

    if (filter_.isEmpty()) { return true; }
    auto data = index.data(Qt::DisplayRole);
    return has_match(filter_.toStdString().c_str(), data.toString().toStdString().c_str());
}

bool ProjectProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    auto lhs = static_cast<ProjectItem*>(source_left.internalPointer());
    auto rhs = static_cast<ProjectItem*>(source_right.internalPointer());

    if (lhs->is_folder_ && !rhs->is_folder_) {
        return true;
    } else if (!lhs->is_folder_ && rhs->is_folder_) {
        return false;
    }
    return lhs->name_.compare(rhs->name_, Qt::CaseInsensitive) < 0;
}

void ProjectProxyModel::onFilterChanged(QString filter)
{
    filter_ = std::move(filter);
    invalidateFilter();
}

// == ProjectView ==========================================================
// ============================================================================

ProjectView::ProjectView(nw::StaticDirectory* module, QWidget* parent)
    : ArclightTreeView(parent)
    , module_{module}
{
    setHeaderHidden(true);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setRootIsDecorated(false);
    connect(this, &QTreeView::doubleClicked, this, &ProjectView::onDoubleClicked);
}

ProjectView::~ProjectView()
{
    delete model_;
}

void ProjectView::activateModel()
{
    proxy_ = new ProjectProxyModel(this);
    proxy_->setRecursiveFilteringEnabled(true);
    proxy_->setSourceModel(model_);
    setModel(proxy_);
    proxy_->sort(0);
}

AbstractTreeModel* ProjectView::loadModel()
{
    model_ = new ProjectModel(module_);
    model_->loadRootItems();
    return model_;
}

void ProjectView::onDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) { return; }
    QModelIndex sourceIndex = proxy_->mapToSource(index);
    if (!sourceIndex.isValid()) { return; }

    emit itemDoubleClicked(static_cast<ProjectItem*>(sourceIndex.internalPointer()));
}
