#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DialogView/dialogmodel.h"
#include "DialogView/dialogview.h"
#include "fontchooserdialog.h"
#include "widgets/util/strings.h"

#include "nw/formats/Dialog.hpp"
#include "nw/log.hpp"
#include "nw/serialization/Gff.hpp"

#include "nlohmann/json.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QSettings>
#include <QTreeView>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , font_(QApplication::font())
{
    ui->setupUi(this);

    readSettings();
    font_dialog_ = new FontChooserDialog(this); // Has to be after settings load.

    for (int i = 0; i < 10; ++i) {
        QAction* act = new QAction(this);
        if (i < recentFiles_.size()) {
            ui->menuRecent->addAction(act);
            act->setData(recentFiles_[i]);
            act->setText(to_qstring(fmt::format("&{} - {}", i + 1, recentFiles_[i].toStdString())));
        } else {
            ui->menuRecent->addAction(act);
            act->setVisible(false);
        }
        recentActions_.append(act);
        QObject::connect(act, &QAction::triggered, this, &MainWindow::onActionRecent);
    }

    QObject::connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAbout);
    QObject::connect(ui->actionAboutQt, &QAction::triggered, this, &MainWindow::onActionAboutQt);

    QObject::connect(ui->actionClose, &QAction::triggered, this, &MainWindow::onActionClose);
    QObject::connect(ui->actionFont, &QAction::triggered, this, &MainWindow::onActionFont);
    QObject::connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onActionNew);
    QObject::connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpen);
    QObject::connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onActionSave);
    QObject::connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onActionSaveAs);

    QObject::connect(ui->actionAdd, &QAction::triggered, this, &MainWindow::onActionAdd);
    QObject::connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::onActionCopy);
    QObject::connect(ui->actionCut, &QAction::triggered, this, &MainWindow::onActionCut);
    QObject::connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::onActionPaste);
    QObject::connect(ui->actionPasteAsLink, &QAction::triggered, this, &MainWindow::onActionPasteAsLink);
    QObject::connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::onActionDelete);

    QObject::connect(ui->actionEnglish_2, &QAction::triggered, this, &MainWindow::onActionLangauge);
    QObject::connect(ui->actionFrench_2, &QAction::triggered, this, &MainWindow::onActionLangauge);
    QObject::connect(ui->actionGerman_2, &QAction::triggered, this, &MainWindow::onActionLangauge);
    QObject::connect(ui->actionPolish_2, &QAction::triggered, this, &MainWindow::onActionLangauge);
    QObject::connect(ui->actionSpanish_2, &QAction::triggered, this, &MainWindow::onActionLangauge);
    QObject::connect(ui->actionFeminine, &QAction::triggered, this, &MainWindow::onActionLangaugeFeminine);

    QObject::connect(ui->dialogTabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    QObject::connect(font_dialog_->buttonBox(), &QDialogButtonBox::accepted, this, &MainWindow::onFontAccepted);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Methods

DialogView* MainWindow::current()
{
    return reinterpret_cast<DialogView*>(ui->dialogTabWidget->currentWidget());
}

void MainWindow::open(const QString& path)
{
    if (!QFileInfo::exists(path)) { return; }
    for (int i = 0; i < ui->dialogTabWidget->count(); ++i) {
        auto cw = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(i));
        if (!cw) { continue; }
        if (path == cw->path()) {
            ui->dialogTabWidget->setCurrentIndex(i);
            return;
        }
    }

    QFileInfo fileInfo(path);
    auto ext = fileInfo.completeSuffix();
    nw::Dialog* dlg = nullptr;

    if (0 == ext.compare("dlg", Qt::CaseInsensitive)) {
        nw::Gff gff(path.toStdString());
        if (!gff.valid()) {
            LOG_F(ERROR, "[dlg] failed to open file: {}", path.toStdString());
            return;
        }
        dlg = new nw::Dialog(gff.toplevel());
    } else if (0 == ext.compare("dlg.json", Qt::CaseInsensitive)) {
        dlg = new nw::Dialog();
        nlohmann::json j;
        std::ifstream f{path.toStdString()};
        f >> j;
        nw::deserialize(j, *dlg);
    } else {
        // [TODO] Warning box
        return;
    }

    if (!dlg->valid()) {
        LOG_F(ERROR, "[dlg] invalid dialog: {}", path.toStdString());
        delete dlg;
        return;
    }

    updateRecent(path);

    auto tv = new DialogView(path);
    auto model = new DialogModel(dlg, tv);
    model->loadRootItems();
    connect(this, &MainWindow::languageChanged, tv, &DialogView::onLanguageChanged);
    connect(tv, &DialogView::dataChanged, this, &MainWindow::onDialogDataChanged);

    tv->setModel(model);
    tv->onLanguageChanged(lang_, feminine_);
    tv->setColors(entry_color_, reply_color_, link_color_);
    tv->setFont(font_);
    tv->selectFirst();

    QString filename(fileInfo.fileName());
    int idx = ui->dialogTabWidget->addTab(tv, filename);
    ui->dialogTabWidget->setTabsClosable(true);
    ui->dialogTabWidget->setCurrentIndex(idx);
}

void MainWindow::readSettings()
{
    QSettings settings("jmd", "dlg");
    int size = settings.beginReadArray("Recent Files");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        recentFiles_.append(settings.value("file").toString());
    }
    settings.endArray();

    auto entry_color = settings.value("colors/entry");
    if (!entry_color.isNull()) {
        entry_color_ = QColor(entry_color.toString());
    }

    auto reply_color = settings.value("colors/reply");
    if (!reply_color.isNull()) {
        reply_color_ = QColor(reply_color.toString());
    }

    auto link_color = settings.value("colors/link");
    if (!link_color.isNull()) {
        link_color_ = QColor(link_color.toString());
    }

    auto font = settings.value("font");
    if (!font.isNull()) {
        font_.fromString(font.toString());
    }
}

void MainWindow::restoreWindow()
{
    QSettings settings("jmd", "dlg");
    auto geom = settings.value("Window/geometry");
    if (!geom.isNull()) {
        restoreGeometry(geom.toByteArray());
    }
}

void MainWindow::setModifiedTabName(bool modified)
{
    if (!current()) { return; }
    auto name = current()->name();
    if (modified) {
        ui->dialogTabWidget->setTabText(ui->dialogTabWidget->currentIndex(), name.isEmpty() ? "untitled*" : name + "*");
    } else {
        ui->dialogTabWidget->setTabText(ui->dialogTabWidget->currentIndex(), name);
    }
}

void MainWindow::updateRecent(const QString& path)
{
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
}

void MainWindow::writeSettings()
{
    QSettings settings("jmd", "dlg");
    settings.beginWriteArray("Recent Files", static_cast<int>(recentFiles_.size()));
    int i = 0;
    for (const auto& f : recentFiles_) {
        settings.setArrayIndex(i++);
        settings.setValue("file", f);
    }
    settings.endArray();
    settings.setValue("Window/geometry", saveGeometry());

    settings.setValue("colors/entry", entry_color_.name());
    settings.setValue("colors/reply", reply_color_.name());
    settings.setValue("colors/link", link_color_.name());
    settings.setValue("font", font_.toString());
}

// -- Overrides ---------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
}

// -- Public Slots ------------------------------------------------------------

void MainWindow::onActionAbout()
{
    QMessageBox::about(nullptr, "dlg",
        R"end(<h3>About dlg</h3><br>
              A utility for creating/modifying DLG files.<br><br>

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

void MainWindow::onActionAdd()
{
    auto tab = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(ui->dialogTabWidget->currentIndex()));
    if (!tab) { return; }
    tab->onDialogAddNode();
}

void MainWindow::onActionClose()
{
    auto index = ui->dialogTabWidget->currentIndex();
    onTabCloseRequested(index);
}

void MainWindow::onActionCopy()
{
    auto tab = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(ui->dialogTabWidget->currentIndex()));
    if (!tab) { return; }
    tab->onDialogCopyNode();
}

void MainWindow::onActionCut()
{
    auto tab = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(ui->dialogTabWidget->currentIndex()));
    if (!tab) { return; }
    tab->onDialogCutNode();
}

void MainWindow::onActionDelete()
{
    auto tab = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(ui->dialogTabWidget->currentIndex()));
    if (!tab) { return; }
    tab->onDialogDeleteNode();
}

void MainWindow::onActionFont()
{
    font_dialog_->entry_color = entry_color_;
    font_dialog_->reply_color = reply_color_;
    font_dialog_->link_color = link_color_;
    font_dialog_->font = font_;
    font_dialog_->update();
    font_dialog_->show();
}

void MainWindow::onFontAccepted()
{
    font_ = font_dialog_->font;
    entry_color_ = font_dialog_->entry_color;
    reply_color_ = font_dialog_->reply_color;
    link_color_ = font_dialog_->link_color;

    for (int i = 0; i < ui->dialogTabWidget->count(); ++i) {
        if (auto view = dynamic_cast<DialogView*>(ui->dialogTabWidget->widget(i))) {
            view->setColors(entry_color_, reply_color_, link_color_);
            view->setFont(font_);
        }
    }
}

void MainWindow::onActionLangauge()
{
    auto action = qobject_cast<QAction*>(sender());
    bool disable_fem = false;

    // [TODO] Clean this up
    if (!action) { return; }
    if (action->text() == "English") {
        disable_fem = true;
        lang_ = nw::LanguageID::english;
        feminine_ = false;
        ui->actionFrench_2->setChecked(false);
        ui->actionGerman_2->setChecked(false);
        ui->actionPolish_2->setChecked(false);
        ui->actionSpanish_2->setChecked(false);
    } else if (action->text() == "French") {
        lang_ = nw::LanguageID::french;
        ui->actionEnglish_2->setChecked(false);
        ui->actionGerman_2->setChecked(false);
        ui->actionPolish_2->setChecked(false);
        ui->actionSpanish_2->setChecked(false);
    } else if (action->text() == "German") {
        lang_ = nw::LanguageID::german;
        ui->actionEnglish_2->setChecked(false);
        ui->actionFrench_2->setChecked(false);
        ui->actionPolish_2->setChecked(false);
        ui->actionSpanish_2->setChecked(false);
    } else if (action->text() == "Polish") {
        lang_ = nw::LanguageID::polish;
        ui->actionEnglish_2->setChecked(false);
        ui->actionGerman_2->setChecked(false);
        ui->actionFrench_2->setChecked(false);
        ui->actionSpanish_2->setChecked(false);
    } else if (action->text() == "Spanish") {
        lang_ = nw::LanguageID::polish;
        ui->actionEnglish_2->setChecked(false);
        ui->actionGerman_2->setChecked(false);
        ui->actionFrench_2->setChecked(false);
        ui->actionPolish_2->setChecked(false);
    }

    ui->actionFeminine->setDisabled(disable_fem);
    if (disable_fem) {
        ui->actionFeminine->setChecked(false);
    }

    emit languageChanged(lang_, feminine_);
}

void MainWindow::onActionLangaugeFeminine()
{
    feminine_ = ui->actionFeminine->isChecked();
    emit languageChanged(lang_, feminine_);
}

void MainWindow::onActionNew()
{
    nw::Dialog* dlg = new nw::Dialog;

    auto tv = new DialogView("");
    auto model = new DialogModel(dlg, tv);
    model->loadRootItems();
    connect(this, &MainWindow::languageChanged, tv, &DialogView::onLanguageChanged);
    connect(tv, &DialogView::dataChanged, this, &MainWindow::onDialogDataChanged);

    tv->setModel(model);
    tv->onLanguageChanged(lang_, feminine_);
    tv->setColors(entry_color_, reply_color_, link_color_);
    tv->setFont(font_);
    tv->selectFirst();

    int idx = ui->dialogTabWidget->addTab(tv, tv->name());
    ui->dialogTabWidget->setTabsClosable(true);
    ui->dialogTabWidget->setCurrentIndex(idx);
    ui->actionSave->setEnabled(false);
}

void MainWindow::onActionOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open Dialog", "", "Dlg (*.dlg *.dlg.json)");
    if (!fn.isEmpty()) {
        open(fn);
    }
}

void MainWindow::onActionPaste()
{
    auto tab = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(ui->dialogTabWidget->currentIndex()));
    if (!tab) { return; }
    tab->onDialogPasteNode();
}

void MainWindow::onActionPasteAsLink()
{
    auto tab = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(ui->dialogTabWidget->currentIndex()));
    if (!tab) { return; }
    tab->onDialogPasteLinkNode();
}

void MainWindow::onActionRecent()
{
    QAction* act = reinterpret_cast<QAction*>(sender());
    open(act->data().toString());
}

void MainWindow::onActionSave()
{
    auto tab = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(ui->dialogTabWidget->currentIndex()));
    if (!tab) { return; }
    tab->onDialogSave();
}

void MainWindow::onActionSaveAs()
{
    auto index = ui->dialogTabWidget->currentIndex();
    auto tab = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(index));
    if (!tab) { return; }
    tab->onDialogSaveAs();

    QFileInfo fileInfo(tab->path());
    ui->dialogTabWidget->setTabText(index, fileInfo.fileName());
    ui->actionSave->setEnabled(!tab->path().isEmpty());
    if (!tab->path().isEmpty()) {
        updateRecent(tab->path());
    }
}

void MainWindow::onDialogDataChanged(bool changed)
{
    setModifiedTabName(changed);
}

void MainWindow::onTabCloseRequested(int index)
{
    auto cw = reinterpret_cast<DialogView*>(ui->dialogTabWidget->widget(index));
    if (cw->modified()) {
        if (QMessageBox::No == QMessageBox::warning(this, "You have unsaved changes!", QString("Are you sure you want to close %1?").arg(cw->name()), QMessageBox::Yes | QMessageBox::No)) {
            return;
        }
    }
    ui->dialogTabWidget->removeTab(index);
    delete cw;
    // if (!current()) {
    //     enableModificationMenus(false);
    // }
}
