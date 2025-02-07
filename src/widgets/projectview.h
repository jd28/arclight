#pragma once

#include "AbstractTreeModel.hpp"

#include "arclighttreeview.h"

#include "nw/resources/StaticDirectory.hpp"
#include "sqlite3.h"

#include <QDateTime>
#include <QFileSystemWatcher>
#include <QSortFilterProxyModel>
#include <QTreeView>

struct ProjectItemMetadata {
    QString object_name;
    qint64 size;
    QDateTime lastModified;
};

// == ProjectItem =============================================================
// ============================================================================

class ProjectItem : public AbstractTreeItem {
public:
    ProjectItem(const QString& name, const QString& path, nw::StaticDirectory* module, ProjectItem* parent = nullptr);
    ProjectItem(const QString& name, const QString& path, nw::Resource res, nw::StaticDirectory* module, ProjectItem* parent = nullptr);
    virtual QVariant data(int column, int role = Qt::DisplayRole) const override;

    nw::StaticDirectory* module_ = nullptr;
    QString path_;
    QString name_;
    nw::Resource res_;
    bool is_folder_ = false;
};

// == ProjectModel ============================================================
// ============================================================================

class ProjectModel : public AbstractTreeModel {
    Q_OBJECT
public:
    explicit ProjectModel(nw::StaticDirectory* module, QObject* parent = nullptr);

    ProjectItemMetadata getMetadata(const QString& path);
    void insertMetadata(const QString& path, const ProjectItemMetadata& metadata);
    bool setupDatabase();
    void walkDirectory(const QString& path, ProjectItem* parent = nullptr);

    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual void loadRootItems() override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;

    QFileSystemWatcher watcher_;
    nw::StaticDirectory* module_ = nullptr;
    QString path_;
    sqlite3* db_ = nullptr;
};

// == ProjectProxyModel =======================================================
// ============================================================================

class ProjectProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    ProjectProxyModel(QObject* parent = nullptr);

protected:
    virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

public slots:
    void onFilterChanged(QString filter);

public:
    QString filter_;
};

// == ProjectView =============================================================
// ============================================================================

class ProjectView : public ArclightTreeView {
    Q_OBJECT
public:
    ProjectView(nw::StaticDirectory* module, QWidget* parent = nullptr);
    ~ProjectView();

    void activateModel();
    AbstractTreeModel* loadModel();

public slots:
    void onDoubleClicked(const QModelIndex& index);

signals:
    void itemDoubleClicked(ProjectItem*);

public:
    nw::StaticDirectory* module_ = nullptr;
    ProjectModel* model_;
    ProjectProxyModel* proxy_;
};
