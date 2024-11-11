#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "LanguageMenu/LanguageMenu.h"
#include "widgets/util/strings.h"

#include <nw/log.hpp>
#include <nw/resources/Erf.hpp>

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

#include <filesystem>
namespace fs = std::filesystem;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
{
    readSettings();

    ui_->setupUi(this);
    // auto la = new LanguageMenu(ui_->menubar);
    // la->setTitle(tr("&Language"));
    // ui_->menubar->insertMenu(ui_->menuView->menuAction(), la);

    for (int i = 0; i < 10; ++i) {
        QAction* act = new QAction(this);
        if (i < recentFiles_.size()) {
            ui_->menuRecent->addAction(act);
            act->setData(recentFiles_[i]);
            act->setText(to_qstring(fmt::format("&{} - {}", i + 1, recentFiles_[i].toStdString())));
        } else {
            ui_->menuRecent->addAction(act);
            act->setVisible(false);
        }
        recentActions_.append(act);
        QObject::connect(act, &QAction::triggered, this, &MainWindow::onActionRecent);
    }

    QObject::connect(ui_->actionNew, &QAction::triggered, this, &MainWindow::onActionNew);
    QObject::connect(ui_->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpen);
    QObject::connect(ui_->actionClose, &QAction::triggered, this, &MainWindow::onActionClose);
    QObject::connect(ui_->actionSave, &QAction::triggered, this, &MainWindow::onActionSave);
    QObject::connect(ui_->actionSave_As, &QAction::triggered, this, &MainWindow::onActionSaveAs);

    QObject::connect(ui_->actionImport, &QAction::triggered, this, &MainWindow::onActionImport);
    QObject::connect(ui_->actionDelete_2, &QAction::triggered, this, &MainWindow::onActionDelete);
    QObject::connect(ui_->actionExport, &QAction::triggered, this, &MainWindow::onActionExport);
    QObject::connect(ui_->actionExport_All, &QAction::triggered, this, &MainWindow::onActionExportAll);
    QObject::connect(ui_->actionMerge, &QAction::triggered, this, &MainWindow::onActionMerge);

    QObject::connect(ui_->actionAbout, &QAction::triggered, this, &MainWindow::onActionAbout);
    QObject::connect(ui_->actionAbout_Qt_2, &QAction::triggered, this, &MainWindow::onActionAboutQt);

    QObject::connect(ui_->containerTabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
}

MainWindow::~MainWindow()
{
    delete ui_;
}

ContainerView* MainWindow::current()
{
    return reinterpret_cast<ContainerView*>(ui_->containerTabWidget->currentWidget());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::open(const QString& path)
{
    if (!QFileInfo::exists(path)) { return; }
    std::string p = path.toStdString();
    for (int i = 0; i < ui_->containerTabWidget->count(); ++i) {
        auto cw = reinterpret_cast<ContainerView*>(ui_->containerTabWidget->widget(i));
        if (!cw) { continue; }
        if (p == cw->container()->path()) {
            ui_->containerTabWidget->setCurrentIndex(i);
            currentContainer_ = cw;
            return;
        }
    }

    nw::Container* c = new nw::Erf(p);
    currentContainer_ = new ContainerView(c, 2);
    int idx = ui_->containerTabWidget->addTab(currentContainer_, to_qstring(c->name()));
    ui_->containerTabWidget->setTabsClosable(true);
    ui_->containerTabWidget->setCurrentIndex(idx);

    if (recentFiles_.contains(path)) {
        recentFiles_.removeOne(path);
    } else if (recentFiles_.size() >= 10) {
        recentFiles_.pop_back();
    }
    recentFiles_.insert(0, path);

    for (int i = 0; i < recentFiles_.size(); ++i) {
        recentActions_[i]->setData(recentFiles_[i]);
        recentActions_[i]->setText(to_qstring(
            fmt::format("&{} - {}", i + 1, recentFiles_[i].toStdString())));
        recentActions_[i]->setVisible(true);
    }

    connectModifiedSlots(current()->model());
    enableModificationMenus(true);
}

void MainWindow::restoreWindow()
{
    QSettings settings("jmd", "erfherder");
    auto geom = settings.value("Window/geometry");
    if (!geom.isNull()) {
        restoreGeometry(geom.toByteArray());
    }
}

void MainWindow::onActionNew()
{
    nw::Container* c = new nw::Erf();
    currentContainer_ = new ContainerView(c, 2);
    int idx = ui_->containerTabWidget->addTab(currentContainer_, "untitled");
    ui_->containerTabWidget->setTabsClosable(true);
    ui_->containerTabWidget->setCurrentIndex(idx);
    enableModificationMenus(true);
    connectModifiedSlots(current()->model());
}

void MainWindow::onActionOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open Erf", "", "Erf (*.erf *.mod *.hak *.nwm *.sav)");
    if (!fn.isEmpty()) {
        open(fn);
    }
}

void MainWindow::onActionRecent()
{
    QAction* act = reinterpret_cast<QAction*>(sender());
    open(act->data().toString());
}

void MainWindow::onActionImport()
{
    auto files = QFileDialog::getOpenFileNames(this, "Import...");
    if (!files.isEmpty()) {
        current()->model()->addFiles(files);
    }
}

void MainWindow::onActionMerge()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Merge Files...", "", "Erf (*.erf *.mod *.hak)");
    if (!files.isEmpty()) {
        current()->model()->mergeFiles(files);
    }
}

void MainWindow::onActionExport()
{
    if (!current()) { return; }

    auto path = QFileDialog::getExistingDirectory(this, "Export To...");
    if (path.isEmpty()) { return; }

    auto select = current()->table()->selectionModel()->selectedRows();
    for (const auto& s : select) {
        auto idx = current()->model()->index(s.row(), 0);
        auto fname = idx.data(Qt::DisplayRole).toString().toStdString();
        current()->container()->extract_by_glob(fname, path.toStdString());
    }
}

void MainWindow::onActionExportAll()
{
    if (!current()) { return; }

    auto path = QFileDialog::getExistingDirectory(this, "Export To...");
    if (path.isEmpty()) { return; }

    if (auto c = current()->container()) {
        c->extract(std::regex(".*"), path.toStdString());
    }
}

void MainWindow::onActionDelete()
{
    QModelIndexList idxs;
    while ((idxs = current()->table()->selectionModel()->selectedIndexes()).size()) {
        current()->proxy()->removeRow(idxs.first().row());
    }
}

void MainWindow::onActionSave()
{
    if (!current()) { return; }
    if (current()->container()->name().empty()) {
        onActionSaveAs();
    } else if (auto e = dynamic_cast<nw::Erf*>(current()->container())) {
        e->save();
        e->reload();
        setModifiedTabName(false);
    }
}

void MainWindow::onActionSaveAs()
{
    if (!current()) { return; }
    QString fn = QFileDialog::getSaveFileName(this, "Save As", "", "Erf (*.erf *.mod *.hak *.nwm *.sav)");
    if (fn.isEmpty()) { return; }
    if (auto e = dynamic_cast<nw::Erf*>(current()->container())) {
        e->save_as(fn.toStdString());
        onTabCloseRequested(ui_->containerTabWidget->currentIndex());
        open(fn);
    }
}

void MainWindow::onActionClose()
{
    auto index = ui_->containerTabWidget->currentIndex();
    onTabCloseRequested(index);
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(nullptr, "erfherder",
        R"end(<h3>About erfherder</h3><br>
              A utility for creating/modifying ERF files.<br><br>

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)end");
}

void MainWindow::onActionAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::onModelModified(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);
    setModifiedTabName(true);
}

void MainWindow::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    setModifiedTabName(true);
}

void MainWindow::onRowsRemoved(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    setModifiedTabName(true);
}

void MainWindow::onTabCloseRequested(int index)
{
    auto cw = reinterpret_cast<ContainerView*>(ui_->containerTabWidget->widget(index));
    ui_->containerTabWidget->removeTab(index);
    delete cw;
    if (!current()) {
        enableModificationMenus(false);
    }
}

void MainWindow::readSettings()
{
    QSettings settings("jmd", "erfherder");
    int size = settings.beginReadArray("Recent Files");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        recentFiles_.append(settings.value("file").toString());
    }
    settings.endArray();
}

void MainWindow::writeSettings()
{
    QSettings settings("jmd", "erfherder");
    settings.beginWriteArray("Recent Files", static_cast<int>(recentFiles_.size()));
    int i = 0;
    foreach (const auto& f, recentFiles_) {
        settings.setArrayIndex(i++);
        settings.setValue("file", f);
    }
    settings.endArray();
    settings.setValue("Window/geometry", saveGeometry());
}

void MainWindow::connectModifiedSlots(ContainerModel* model)
{
    QObject::connect(model, &QAbstractItemModel::dataChanged,
        this, &MainWindow::onModelModified);
    QObject::connect(model, &QAbstractItemModel::rowsInserted,
        this, &MainWindow::onRowsInserted);
    QObject::connect(model, &QAbstractItemModel::rowsRemoved,
        this, &MainWindow::onRowsRemoved);
}

void MainWindow::enableModificationMenus(bool enabled)
{
    if (current() && dynamic_cast<nw::Erf*>(current()->container())) {
        ui_->actionImport->setEnabled(enabled);
        ui_->actionMerge->setEnabled(enabled);
    } else {
        ui_->actionImport->setEnabled(false);
        ui_->actionMerge->setEnabled(false);
    }
    ui_->actionExport->setEnabled(enabled);
    ui_->actionExport_All->setEnabled(enabled);
}

void MainWindow::setModifiedTabName(bool modified)
{
    auto name = current()->container()->name();
    if (modified) {
        ui_->containerTabWidget->setTabText(ui_->containerTabWidget->currentIndex(),
            to_qstring(name.empty() ? "untitled*" : name + "*"));
    } else {
        ui_->containerTabWidget->setTabText(ui_->containerTabWidget->currentIndex(),
            to_qstring(name));
    }
}
