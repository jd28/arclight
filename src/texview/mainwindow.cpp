#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "nw/log.hpp"

#include "texuregallerymodel.h"
#include "widgets/util/strings.h"

#include <QFileDialog>
#include <QImage>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    readSettings();

    for (int i = 0; i < 10; ++i) {
        QAction* act = new QAction(this);
        if (i < recentFiles_.size()) {
            ui->menuRecent->addAction(act);
            act->setData(recentFiles_[i]);
            act->setText(to_qstring(
                fmt::format("&{} - {}", i + 1, recentFiles_[i].toStdString())));
        } else {
            ui->menuRecent->addAction(act);
            act->setVisible(false);
        }
        recentActions_.append(act);
        QObject::connect(act, &QAction::triggered, this, &MainWindow::onActionRecent);
    }

    QObject::connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAbout);
    QObject::connect(ui->actionAboutQt, &QAction::triggered, this, &MainWindow::onActionAboutQt);
    QObject::connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onActionExit);
    QObject::connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpen);
    QObject::connect(ui->actionOpen_Folder, &QAction::triggered, this, &MainWindow::onActionOpenFolder);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// -- Methods -----------------------------------------------------------------

void MainWindow::open(const QString& path)
{
    if (!QFileInfo::exists(path)) {
        return;
    }

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

    ui->imageGallery->setModel(new TexureGalleryModel(path));
}

void MainWindow::readSettings()
{
    QSettings settings("jmd", "texview");
    int size = settings.beginReadArray("Recent Files");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        recentFiles_.append(settings.value("file").toString());
    }
    settings.endArray();
}

void MainWindow::restoreWindow()
{
    QSettings settings("jmd", "texview");
    auto geom = settings.value("Window/geometry");
    if (!geom.isNull()) {
        restoreGeometry(geom.toByteArray());
    }
}

void MainWindow::writeSettings()
{
    QSettings settings("jmd", "texview");
    settings.beginWriteArray("Recent Files", static_cast<int>(recentFiles_.size()));
    int i = 0;
    for (const auto& f : recentFiles_) {
        settings.setArrayIndex(i++);
        settings.setValue("file", f);
    }
    settings.endArray();
    settings.setValue("Window/geometry", saveGeometry());
}

// -- Overrides ---------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
}

// -- Slots -------------------------------------------------------------------
void MainWindow::onActionAbout()
{
    QMessageBox::about(nullptr, "texview",
        R"end(<h3>About texview</h3><br>
              A texture gallery viewer for NWN textures.<br><br>

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

void MainWindow::onActionExit()
{
    QApplication::quit();
}

void MainWindow::onActionOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open Container", "", "Container (*.erf *.hak *.key *.zip)");
    if (fn.isEmpty()) {
        return;
    }
    open(fn);
}

void MainWindow::onActionOpenFolder()
{
    QString fn = QFileDialog::getExistingDirectory(this, "Open Folder");
    if (fn.isEmpty()) {
        return;
    }
    open(fn);
}

void MainWindow::onActionRecent()
{
    QAction* act = reinterpret_cast<QAction*>(sender());
    open(act->data().toString());
}
