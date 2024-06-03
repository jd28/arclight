#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "nw/resources/Resource.hpp"

#include <absl/container/flat_hash_map.h>

#include <QFutureWatcher>
#include <QMainWindow>

#include <functional>

class ArclightView;
class AreaListItem;
class ProjectItem;
class WaitingSpinnerWidget;

class QTreeView;

namespace nw {
struct Module;
struct StaticDirectory;
}

namespace Ui {
class MainWindow;
}

using ResourceCallback = std::function<ArclightView*(nw::Resource)>;
using ExtensionCallback = std::function<ArclightView*(const std::string&)>;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadCallbacks();
    void loadTreeviews();

public slots:
    void onActionClose(bool checked = false);
    void onActionCloseProject(bool checked = false);
    void onActionOpen(bool checked = false);
    void onAreaListDoubleClicked(AreaListItem* item);
    void onProjectDoubleClicked(ProjectItem* item);
    void onProjectViewChanged(int index);
    void onTabCloseRequested(int index);

private:
    Ui::MainWindow *ui;
    nw::Module* module_ = nullptr;
    nw::StaticDirectory* module_container_ = nullptr;
    QString module_path_;
    absl::flat_hash_map<QString, ExtensionCallback> ext_to_view_;
    absl::flat_hash_map<nw::ResourceType::type, ResourceCallback> type_to_view_;
    QList<QTreeView*> project_treeviews_;
    bool close_project_cancelled_ = false;
    QFuture<QList<nw::Module*>> mod_load_future_;
    QFutureWatcher<QList<nw::Module*>>* mod_load_watcher_ = nullptr;
    WaitingSpinnerWidget* spinner_ = nullptr;
};

#endif // MAINWINDOW_H
