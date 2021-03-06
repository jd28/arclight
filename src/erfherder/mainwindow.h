#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "widgets/ContainerWidget.hpp"

#include <QMainWindow>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void open(const QString& path);
    ContainerWidget* current();
    void restoreWindow();

public slots:
    void onActionNew();
    void onActionOpen();
    void onActionRecent();
    void onActionSave();
    void onActionSaveAs();
    void onActionClose();

    void onActionImport();
    void onActionMerge();
    void onActionExport();
    void onActionExportAll();
    void onActionDelete();

    void onTabCloseRequested(int index);

    void onActionAbout();
    void onActionAboutQt();

    void onModelModified(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onRowsRemoved(const QModelIndex& parent, int first, int last);

    void writeSettings();

private:
    void connectModifiedSlots(ContainerModel* model);
    void enableModificationMenus(bool enabled);
    void setModifiedTabName(bool modified);

    Ui::MainWindow* ui_;
    ContainerWidget* currentContainer_;
    QSettings settings_;
    QStringList recentFiles_;
    QList<QAction*> recentActions_;

    void readSettings();
};

#endif // MAINWINDOW_H
