#include "dialogview.h"
#include "ui_dialogview.h"

#include "../util/strings.h"
#include "dialogmodel.h"
#include "dialogtreeview.h"

#include "nw/formats/Dialog.hpp"
#include "nw/kernel/Resources.hpp"
#include "nw/serialization/GffBuilder.hpp"

#include "nlohmann/json.hpp"

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include <QApplication>
#include <QAudioOutput>
#include <QBuffer>
#include <QColor>
#include <QCompleter>
#include <QFileDialog>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMenu>
#include <QPushButton>
#include <QScreen>

DialogView::DialogView(QString path, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::DialogView)
    , path_{path}
{
    ui->setupUi(this);
    setupUi();
}

DialogView::DialogView(nw::Dialog* dialog, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::DialogView)
    , model_{new DialogModel(dialog, this)}
{
    ui->setupUi(this);
    setupUi();
    model_->loadRootItems();
    setModel(model_);
}

DialogView::~DialogView()
{
    delete ui;
}

void DialogView::loadItem(DialogItem* item)
{
    current_item_ = nullptr;

    // Action Script
    ui->actionScript->setText(to_qstring(item->ptr_ ? item->ptr_->node->script_action.view() : ""));
    ui->actionScript->setDisabled(!item->ptr_ || item->ptr_->is_link);

    // Actions Param Table
    ui->actionParams->clearContents();
    ui->actionParams->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    if (item->ptr_) {
        ui->actionParams->setRowCount(int(item->ptr_->node->action_params.size()));
        ui->actionParams->setColumnCount(2);
        for (size_t r = 0; r < item->ptr_->node->action_params.size(); ++r) {
            ui->actionParams->setItem(int(r), 0, new QTableWidgetItem(to_qstring(item->ptr_->node->action_params[r].first)));
            ui->actionParams->setItem(int(r), 1, new QTableWidgetItem(to_qstring(item->ptr_->node->action_params[r].second)));
        }
    }
    ui->actionParams->setDisabled(!item->ptr_ || item->ptr_->is_link);

    // Action Sound
    ui->actionSound->setText(item->ptr_ ? to_qstring(item->ptr_->node->sound.view()) : "");
    ui->actionSound->setDisabled(!item->ptr_ || item->ptr_->is_link);

    // Action Animation
    ui->actionAnimation->setDisabled(!item->ptr_ || item->ptr_->is_link);
    if (item->ptr_) {
        switch (item->ptr_->node->animation) {
        default:
            break;
        case nw::DialogAnimation::default_:
            ui->actionAnimation->setCurrentIndex(0);
            break;
        case nw::DialogAnimation::none:
            ui->actionAnimation->setCurrentIndex(1);
            break;
        case nw::DialogAnimation::taunt:
            ui->actionAnimation->setCurrentIndex(2);
            break;
        case nw::DialogAnimation::greeting:
            ui->actionAnimation->setCurrentIndex(3);
            break;
        case nw::DialogAnimation::listen:
            ui->actionAnimation->setCurrentIndex(4);
            break;
        case nw::DialogAnimation::worship:
            ui->actionAnimation->setCurrentIndex(5);
            break;
        case nw::DialogAnimation::salute:
            ui->actionAnimation->setCurrentIndex(6);
            break;
        case nw::DialogAnimation::bow:
            ui->actionAnimation->setCurrentIndex(7);
            break;
        case nw::DialogAnimation::steal:
            ui->actionAnimation->setCurrentIndex(8);
            break;
        case nw::DialogAnimation::talk_normal:
            ui->actionAnimation->setCurrentIndex(9);
            break;
        case nw::DialogAnimation::talk_pleading:
            ui->actionAnimation->setCurrentIndex(10);
            break;
        case nw::DialogAnimation::talk_forceful:
            ui->actionAnimation->setCurrentIndex(11);
            break;
        case nw::DialogAnimation::talk_laugh:
            ui->actionAnimation->setCurrentIndex(12);
            break;
        case nw::DialogAnimation::victory_1:
            ui->actionAnimation->setCurrentIndex(13);
            break;
        case nw::DialogAnimation::victory_2:
            ui->actionAnimation->setCurrentIndex(14);
            break;
        case nw::DialogAnimation::victory_3:
            ui->actionAnimation->setCurrentIndex(15);
            break;

        case nw::DialogAnimation::look_far:
            ui->actionAnimation->setCurrentIndex(16);
            break;
        case nw::DialogAnimation::drink:
            ui->actionAnimation->setCurrentIndex(17);
            break;
        case nw::DialogAnimation::read:
            ui->actionAnimation->setCurrentIndex(18);
            break;
        }
    }

    // Condition script
    ui->conditionScript->setText(to_qstring(item->ptr_ ? item->ptr_->script_appears.view() : ""));
    ui->conditionScript->setDisabled(!item->ptr_ || item->ptr_->is_link);

    // Condition Param Table
    ui->conditionParams->clearContents();
    ui->conditionParams->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    if (item->ptr_) {
        ui->conditionParams->setRowCount(int(item->ptr_->condition_params.size()));
        ui->conditionParams->setColumnCount(2);
        for (size_t r = 0; r < item->ptr_->condition_params.size(); ++r) {
            ui->conditionParams->setItem(int(r), 0, new QTableWidgetItem(to_qstring(item->ptr_->condition_params[r].first)));
            ui->conditionParams->setItem(int(r), 1, new QTableWidgetItem(to_qstring(item->ptr_->condition_params[r].second)));
        }
    }
    ui->conditionParams->setDisabled(!item->ptr_ || item->ptr_->is_link);

    // Dialog Text
    ui->dialogTextEdit->setCurrentFont(font_);
    ui->dialogTextEdit->setText(item->ptr_ ? to_qstring(item->ptr_->node->text.get(lang_, feminine_)) : "");
    ui->dialogTextEdit->setDisabled(!item->ptr_ || item->ptr_->is_link);

    // Exit Scripts
    ui->abortLineEdit->setText(to_qstring(item->ptr_ ? item->ptr_->parent->script_abort.view() : ""));
    ui->endLineEdit->setText(to_qstring(item->ptr_ ? item->ptr_->parent->script_end.view() : ""));

    // Speaker Table
    if (!item->ptr_ || !item->ptr_->is_link) {
        ui->speakersTable->setDisabled(!item->ptr_ || item->ptr_->type == nw::DialogNodeType::reply);
    }

    if (item->ptr_) {
        int select_speaker = 0;
        if (!item->ptr_->node->speaker.empty()) {
            for (int i = 0; i < ui->speakersTable->rowCount(); ++i) {
                auto twi = ui->speakersTable->item(i, 0);
                auto twi_data = twi->data(Qt::DisplayRole);
                if (!twi_data.isNull() && item->ptr_->node->speaker == twi_data.toString().toStdString()) {
                    select_speaker = i;
                }
            }
        }
        ui->speakersTable->setCurrentCell(select_speaker, 0);
    }

    // Comment
    ui->commentText->setCurrentFont(font_);
    if (!item->ptr_) {
        ui->commentText->setText("");
    }
    else if (item->ptr_->is_link) {
        ui->commentText->setText(to_qstring(item->ptr_->comment));
    } else {
        ui->commentText->setText(to_qstring(item->ptr_->node->comment));
    }
    ui->commentText->setDisabled(!item->ptr_);

    current_item_ = item;
}

bool DialogView::modified() const
{
    return modified_;
}

QString DialogView::name() const
{
    if (path_.isEmpty()) { return "untitled"; }
    QFileInfo fileInfo{path_};
    return fileInfo.fileName();
}

void DialogView::selectFirst()
{
    auto index = model_->index(0, 0);
    ui->dialogView->expandRecursively(index);
    ui->dialogView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    auto new_ptr = reinterpret_cast<DialogItem*>(index.internalPointer());
    if (!new_ptr || new_ptr == current_item_) { return; }
    loadItem(new_ptr);
}

void DialogView::setColors(QColor entry, QColor reply, QColor link)
{
    model_->setColors(entry, reply, link);
}

void DialogView::setFont(const QFont& font)
{
    font_ = font;
    ui->dialogTextEdit->setFont(font_);
    model_->setFont(font_);
}

void DialogView::setModel(DialogModel* model)
{
    model_ = model;
    ui->dialogView->setModel(model);
    connect(ui->dialogView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogView::onSelectionChanged);
}

void DialogView::setModified(bool modified)
{
    modified_ = modified;
    emit dataChanged(modified);
}

void DialogView::setupUi()
{
    context_menu_ = new QMenu(this);
    add_ctx_action_ = context_menu_->addAction("Add");
    add_ctx_action_->setIcon(QIcon::fromTheme(QString::fromUtf8("list-add")));
    context_menu_->addSeparator();
    copy_ctx_action_ = context_menu_->addAction("Copy");
    copy_ctx_action_->setIcon(QIcon::fromTheme(QString::fromUtf8("edit-copy")));
    cut_ctx_action_ = context_menu_->addAction("Cut");
    cut_ctx_action_->setIcon(QIcon::fromTheme(QString::fromUtf8("edit-cut")));
    context_menu_->addSeparator();
    paste_ctx_action_ = context_menu_->addAction("Paste");
    paste_ctx_action_->setIcon(QIcon::fromTheme(QString::fromUtf8("edit-paste")));
    paste_link_ctx_action_ = context_menu_->addAction("Paste As Link");
    paste_link_ctx_action_->setIcon(QIcon::fromTheme(QString::fromUtf8("insert-link")));
    context_menu_->addSeparator();
    delete_ctx_action_ = context_menu_->addAction("Delete");
    delete_ctx_action_->setIcon(QIcon::fromTheme(QString::fromUtf8("edit-delete")));

    connect(add_ctx_action_, &QAction::triggered, this, &DialogView::onDialogAddNode);
    connect(copy_ctx_action_, &QAction::triggered, this, &DialogView::onDialogCopyNode);
    connect(cut_ctx_action_, &QAction::triggered, this, &DialogView::onDialogCutNode);
    connect(paste_ctx_action_, &QAction::triggered, this, &DialogView::onDialogPasteNode);
    connect(paste_link_ctx_action_, &QAction::triggered, this, &DialogView::onDialogPasteLinkNode);
    connect(delete_ctx_action_, &QAction::triggered, this, &DialogView::onDialogDeleteNode);

    ui->actionsButton->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_code));
    ui->languageButton->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_language));

    // Action Script
    connect(ui->actionScript, &QLineEdit::textChanged, this, &DialogView::onActionScriptChanged);
    connect(ui->actionAnimation, &QComboBox::currentIndexChanged, this, &DialogView::onActionAnimationChanged);

    // Action Param
    connect(ui->actionParams, &QTableWidget::cellChanged, this, &DialogView::onActionParamCellChanged);
    ui->actionParamAdd->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_plus, Qt::green));
    ui->actionParamDel->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_minus, Qt::red));
    connect(ui->actionParamAdd, &QPushButton::clicked, this, &DialogView::onActionParamAddClicked);
    connect(ui->actionParamDel, &QPushButton::clicked, this, &DialogView::onActionParamDelClicked);

    // Action Sound
    connect(ui->actionSound, &QLineEdit::textChanged, this, &DialogView::onActionSoundChanged);
    connect(ui->actionSoundPlay, &QPushButton::clicked, this, &DialogView::onActionSoundClicked);

    // Condition Script
    connect(ui->conditionScript, &QLineEdit::textChanged, this, &DialogView::onConditionScriptChanged);

    // Condition Param
    ui->conditionParamsAdd->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_plus, Qt::green));
    ui->conditionParamsDel->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_minus, Qt::red));
    connect(ui->conditionParamsAdd, &QPushButton::clicked, this, &DialogView::onConditionParamAddClicked);
    connect(ui->conditionParamsDel, &QPushButton::clicked, this, &DialogView::onConditionParamDelClicked);
    connect(ui->conditionParams, &QTableWidget::cellChanged, this, &DialogView::onConditionParamCellChanged);

    // Dialog Text
    connect(ui->dialogTextEdit, &QTextEdit::textChanged, this, &DialogView::onDialogTextChanged);

    // Dialog View
    connect(ui->dialogView, &DialogTreeView::customContextMenuRequested, this, &DialogView::onCustomContextMenu);

    // Speakers table
    ui->speakersListAdd->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_plus, Qt::green));
    ui->speakersListDel->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_minus, Qt::red));
    connect(ui->speakersListAdd, &QPushButton::clicked, this, &DialogView::onSpeakersListAddClicked);
    connect(ui->speakersListDel, &QPushButton::clicked, this, &DialogView::onSpeakersListDelClicked);
    connect(ui->speakersTable, &QTableWidget::cellChanged, this, &DialogView::onSpeakerTableCellChanged);

    // DLG wide Settings
    connect(ui->noZoom, &QCheckBox::stateChanged, this, &DialogView::onNoZoomChanged);
    connect(ui->abortLineEdit, &QLineEdit::textChanged, this, &DialogView::onAbortScriptChanged);
    connect(ui->endLineEdit, &QLineEdit::textChanged, this, &DialogView::onEndScriptChanged);

    // Comment
    connect(ui->commentText, &QTextEdit::textChanged, this, &DialogView::onCommentTextChanged);

    // [TODO] persist these values
    auto width = qApp->primaryScreen()->geometry().width();
    auto height = qApp->primaryScreen()->geometry().height();
    ui->splitter->setSizes(QList<int>() << width * 3 / 4 << width * 1 / 4);
    ui->splitter_2->setSizes(QList<int>() << height * 3 / 4 << height * 1 / 4);

    QStringList scripts;
    auto get_scripts = [&scripts](const nw::Resource& res) {
        // Note: we only care about compiled scripts (for now).
        if (res.type != nw::ResourceType::ncs) { return; }
        scripts << to_qstring(res.resref.string());
    };

    nw::kernel::resman().visit(get_scripts);

    scripts.sort(Qt::CaseInsensitive);
    script_completer_ = new QCompleter(scripts, this);
    ui->actionScript->setCompleter(script_completer_);
    ui->conditionScript->setCompleter(script_completer_);
    ui->abortLineEdit->setCompleter(script_completer_);
    ui->endLineEdit->setCompleter(script_completer_);

    QStringList sounds;
    auto get_sounds = [&sounds](const nw::Resource& res) {
        if (!nw::ResourceType::check_category(nw::ResourceType::sound, res.type)) { return; }
        sounds << to_qstring(res.resref.view());
    };

    nw::kernel::resman().visit(get_sounds);
    sounds.sort(Qt::CaseInsensitive);
    sound_completer_ = new QCompleter(sounds, this);
    ui->actionSound->setCompleter(sound_completer_);
}

// Slots

void DialogView::onAbortScriptChanged(const QString& value)
{
    if (!current_item_) { return; }
    current_item_->ptr_->parent->script_abort = nw::Resref{value.toStdString()};
    setModified(true);
}

void DialogView::onActionAnimationChanged(int index)
{
    if (!current_item_) { return; }
    switch (index) {
    default:
        return;
    case 0:
        current_item_->ptr_->node->animation = nw::DialogAnimation::default_;
        break;
    case 1:
        current_item_->ptr_->node->animation = nw::DialogAnimation::none;
        break;
    case 2:
        current_item_->ptr_->node->animation = nw::DialogAnimation::taunt;
        break;
    case 3:
        current_item_->ptr_->node->animation = nw::DialogAnimation::greeting;
        break;
    case 4:
        current_item_->ptr_->node->animation = nw::DialogAnimation::listen;
        break;
    case 5:
        current_item_->ptr_->node->animation = nw::DialogAnimation::worship;
        break;
    case 6:
        current_item_->ptr_->node->animation = nw::DialogAnimation::salute;
        break;
    case 7:
        current_item_->ptr_->node->animation = nw::DialogAnimation::bow;
        break;
    case 8:
        current_item_->ptr_->node->animation = nw::DialogAnimation::steal;
        break;
    case 9:
        current_item_->ptr_->node->animation = nw::DialogAnimation::talk_normal;
        break;
    case 10:
        current_item_->ptr_->node->animation = nw::DialogAnimation::talk_pleading;
        break;
    case 11:
        current_item_->ptr_->node->animation = nw::DialogAnimation::talk_forceful;
        break;
    case 12:
        current_item_->ptr_->node->animation = nw::DialogAnimation::talk_laugh;
        break;
    case 13:
        current_item_->ptr_->node->animation = nw::DialogAnimation::victory_1;
        break;
    case 14:
        current_item_->ptr_->node->animation = nw::DialogAnimation::victory_2;
        break;
    case 15:
        current_item_->ptr_->node->animation = nw::DialogAnimation::victory_3;
        break;
    case 16:
        current_item_->ptr_->node->animation = nw::DialogAnimation::look_far;
        break;
    case 17:
        current_item_->ptr_->node->animation = nw::DialogAnimation::drink;
        break;
    case 18:
        current_item_->ptr_->node->animation = nw::DialogAnimation::read;
        break;
    }
    setModified(true);
}

void DialogView::onActionParamAddClicked()
{
    if (!ui->actionParams->isEnabled()) { return; }

    ui->actionParams->insertRow(ui->actionParams->rowCount());
    ui->actionParams->setFocus();
    ui->actionParams->setCurrentCell(ui->actionParams->rowCount() - 1, 0);
    auto item = ui->actionParams->item(ui->actionParams->rowCount() - 1, 0);
    ui->actionParams->editItem(item);
    setModified(true);
}

void DialogView::onActionParamCellChanged(int row, int col)
{
    if (!current_item_) { return; }
    if (size_t(row) >= current_item_->ptr_->node->action_params.size()) {
        current_item_->ptr_->node->action_params.resize(row + 1);
    }
    auto variant = ui->actionParams->item(row, col)->data(Qt::DisplayRole);
    if (variant.isNull()) { return; }

    auto value = variant.toString().toStdString();

    if (col == 0) {
        current_item_->ptr_->node->action_params[row].first = value;
    } else if (col == 1) {
        current_item_->ptr_->node->action_params[row].second = value;
    }
    setModified(true);
}

void DialogView::onActionParamDelClicked()
{
    if (!ui->actionParams->isEnabled()) { return; }

    QModelIndexList selection = ui->actionParams->selectionModel()->selectedRows();

    for (int i = 0; i < selection.count(); i++) {
        ui->actionParams->removeRow(selection[i].row());
    }
    setModified(true);
}

void DialogView::onActionSoundChanged(const QString& value)
{
    if (!current_item_) { return; }
    current_item_->ptr_->node->sound = nw::Resref{value.toStdString()};
    setModified(true);
}

void DialogView::onActionSoundClicked()
{
    nw::ResourceData rdata;
    nw::Resref sound{ui->actionSound->text().toStdString()};
    for (auto rt : {nw::ResourceType::wav, nw::ResourceType::bmu}) {
        rdata = nw::kernel::resman().demand({sound, rt});
        if (rdata.bytes.size()) { break; }
    }

    if (rdata.bytes.size() == 0) {
        LOG_F(ERROR, "[dlg] failed to load audio file: {}", sound.view());
        return;
    }

    if (!player_) {
        player_ = new QMediaPlayer{this};
        output_ = new QAudioOutput(this);
        player_->setAudioOutput(output_);
    }

    const char* bytes = reinterpret_cast<const char*>(rdata.bytes.data());
    auto size = qsizetype(rdata.bytes.size());
    QUrl url;

    if (rdata.name.type == nw::ResourceType::bmu) {
        if (size < 8) {
            LOG_F(ERROR, "[dlg] invalid bmu file");
            return;
        }
        // Got to strip the BMU tag off, the rest is just mp3.
        if (std::strncmp(bytes, "BMU V1.0", 8) == 0) {
            bytes += 8;
        }
        url = to_qstring(rdata.name.resref.view()) + ".mp3";
    } else {
        url = to_qstring(rdata.name.filename());
    }

    QByteArray ba{bytes, size};
    output_->setVolume(50);

    QBuffer* buffer = new QBuffer(player_);
    buffer->setData(ba);
    if (!buffer->open(QIODevice::ReadOnly)) {
        LOG_F(ERROR, "audio buffer not opened");
    }

    buffer->seek(qint64(0));
    player_->setSourceDevice(buffer, url);
    player_->play();
}

void DialogView::onActionScriptChanged(const QString& value)
{
    if (!current_item_) { return; }
    current_item_->ptr_->node->script_action = nw::Resref{value.toStdString()};
    setModified(true);
}

void DialogView::onCommentTextChanged()
{
    if (!current_item_) { return; }
    if (current_item_->ptr_->is_link) {
        current_item_->ptr_->comment = ui->commentText->toPlainText().toStdString();
    } else {
        current_item_->ptr_->node->comment = ui->commentText->toPlainText().toStdString();
    }
    setModified(true);
}

void DialogView::onConditionParamAddClicked()
{
    if (!ui->conditionParams->isEnabled()) { return; }

    ui->conditionParams->insertRow(ui->conditionParams->rowCount());
    ui->conditionParams->setFocus();
    ui->conditionParams->setCurrentCell(ui->conditionParams->rowCount() - 1, 0);
    auto item = ui->conditionParams->item(ui->conditionParams->rowCount() - 1, 0);
    ui->conditionParams->editItem(item);
}

void DialogView::onConditionParamCellChanged(int row, int col)
{
    if (!current_item_) { return; }
    if (size_t(row) >= current_item_->ptr_->condition_params.size()) {
        current_item_->ptr_->condition_params.resize(row + 1);
    }
    auto variant = ui->conditionParams->item(row, col)->data(Qt::DisplayRole);
    if (variant.isNull()) { return; }

    auto value = variant.toString().toStdString();

    if (col == 0) {
        current_item_->ptr_->condition_params[row].first = value;
    } else if (col == 1) {
        current_item_->ptr_->condition_params[row].second = value;
    }
    setModified(true);
}

void DialogView::onConditionParamDelClicked()
{
    if (!ui->conditionParams->isEnabled()) { return; }

    QModelIndexList selection = ui->conditionParams->selectionModel()->selectedRows();

    for (int i = 0; i < selection.count(); i++) {
        ui->conditionParams->removeRow(selection[i].row());
    }
    if (selection.count() > 0) {
        setModified(true);
    }
}

void DialogView::onConditionScriptChanged(const QString& value)
{
    if (!current_item_) { return; }
    current_item_->ptr_->script_appears = nw::Resref{value.toStdString()};
    setModified(true);
}

void DialogView::onCustomContextMenu(const QPoint& point)
{
    context_index_ = ui->dialogView->indexAt(point);
    if (!context_index_.isValid()) { return; }

    context_menu_->exec(ui->dialogView->viewport()->mapToGlobal(point));
}

void DialogView::onDialogAddNode()
{
    QModelIndexList selection = ui->dialogView->selectionModel()->selectedRows();
    if (selection.empty()) { return; }
    auto index = selection[0];
    if (!index.isValid()) { return; }

    if (!index.isValid()) { return; }

    auto item = reinterpret_cast<DialogItem*>(index.internalPointer());
    nw::DialogPtr* ptr;

    if (!item->parent_) {
        ptr = model_->dialog()->add();
    } else {
        ptr = item->ptr_->add();
    }
    DialogItem* new_item = new DialogItem(ptr, int(item->children_.size()), model_, item);
    model_->addRow(new_item, index);
    QModelIndex new_index = model_->index(int(item->children_.size() - 1), 0, index);
    ui->dialogView->setCurrentIndex(new_index);
    loadItem(new_item);
    ui->dialogTextEdit->setFocus();
    ui->dialogTextEdit->setCurrentFont(font_);
    ui->dialogTextEdit->setText("<< Enter text here >>");
    ui->dialogTextEdit->selectAll();
    setModified(true);
}

void DialogView::onDialogCopyNode()
{
    if (last_copy_or_cut_ && last_edit_was_cut_) {
        model_->dialog()->delete_ptr(last_copy_or_cut_);
    }
    last_copy_or_cut_ = nullptr;

    QModelIndexList selection = ui->dialogView->selectionModel()->selectedRows();
    if (selection.empty()) { return; }
    auto index = selection[0];
    if (!index.isValid()) { return; }

    auto item = reinterpret_cast<DialogItem*>(index.internalPointer());
    last_edit_was_cut_ = false;
    last_copy_or_cut_ = item->ptr_;
}

void DialogView::onDialogCutNode()
{
    if (last_copy_or_cut_ && last_edit_was_cut_) {
        model_->dialog()->delete_ptr(last_copy_or_cut_);
    }
    last_copy_or_cut_ = nullptr;

    QModelIndexList selection = ui->dialogView->selectionModel()->selectedRows();
    if (selection.empty()) { return; }
    auto index = selection[0];
    if (!index.isValid()) { return; }

    auto item = reinterpret_cast<DialogItem*>(index.internalPointer());
    auto parent_item = reinterpret_cast<DialogItem*>(item->parent_);
    model_->deleteRow(item, index.parent());
    model_->deleteAllMatchingRows([item](AbstractTreeItem* cursor) {
        auto check = reinterpret_cast<DialogItem*>(cursor);
        return check->ptr_->is_link && check->ptr_->node == item->ptr_->node;
    });

    if (!parent_item->ptr_) {
        model_->dialog()->remove_ptr(item->ptr_);
    } else {
        parent_item->ptr_->remove_ptr(item->ptr_);
    }

    last_copy_or_cut_ = item->ptr_;
    last_edit_was_cut_ = true;
    delete item;
    setModified(true);
}

void DialogView::onDialogDeleteNode()
{
    QModelIndexList selection = ui->dialogView->selectionModel()->selectedRows();
    if (selection.empty()) { return; }
    auto index = selection[0];
    if (!index.isValid()) { return; }

    auto item = reinterpret_cast<DialogItem*>(index.internalPointer());
    auto parent_item = reinterpret_cast<DialogItem*>(item->parent_);
    if (!parent_item) { return; } // The root

    model_->deleteRow(item, index.parent());
    model_->deleteAllMatchingRows([item](AbstractTreeItem* cursor) {
        auto check = reinterpret_cast<DialogItem*>(cursor);
        return check->ptr_->is_link && check->ptr_->node == item->ptr_->node;
    });

    if (!parent_item->ptr_) {
        model_->dialog()->remove_ptr(item->ptr_);
    } else {
        parent_item->ptr_->remove_ptr(item->ptr_);
    }

    model_->dialog()->delete_ptr(item->ptr_);
    delete item;
    setModified(true);
}

void DialogView::onDialogPasteNode()
{
    if (!last_copy_or_cut_) { return; }

    QModelIndexList selection = ui->dialogView->selectionModel()->selectedRows();
    if (selection.empty()) { return; }
    auto index = selection[0];
    if (!index.isValid()) { return; }

    auto item = reinterpret_cast<DialogItem*>(index.internalPointer());
    if (item->ptr_->type == last_copy_or_cut_->type) { return; }

    nw::DialogPtr* ptr;

    if (!item->parent_) {
        auto temp = last_edit_was_cut_ ? last_copy_or_cut_ : last_copy_or_cut_->copy();
        ptr = model_->dialog()->add_ptr(temp);
    } else {
        auto temp = last_edit_was_cut_ ? last_copy_or_cut_ : last_copy_or_cut_->copy();
        ptr = item->ptr_->add_ptr(temp);
    }

    if (last_edit_was_cut_) {
        last_copy_or_cut_ = nullptr;
    }

    DialogItem* new_item = new DialogItem(ptr, int(item->children_.size()), model_, item);
    model_->addRow(new_item, index);
    QModelIndex new_index = model_->index(int(item->children_.size() - 1), 0, index);
    ui->dialogView->setCurrentIndex(new_index);
    loadItem(new_item);
    ui->dialogTextEdit->setFocus();
    setModified(true);
}

void DialogView::onDialogPasteLinkNode()
{
    if (!last_copy_or_cut_ || last_edit_was_cut_) { return; }
    QModelIndexList selection = ui->dialogView->selectionModel()->selectedRows();
    if (selection.empty()) { return; }
    auto index = selection[0];
    if (!index.isValid()) { return; }

    auto item = reinterpret_cast<DialogItem*>(index.internalPointer());
    if (item->ptr_->type == last_copy_or_cut_->type) { return; }
    nw::DialogPtr* ptr;

    if (!item->parent_) {
        ptr = model_->dialog()->add_ptr(last_copy_or_cut_, true);
    } else {
        ptr = item->ptr_->add_ptr(last_copy_or_cut_, true);
    }
    last_copy_or_cut_ = nullptr;

    DialogItem* new_item = new DialogItem(ptr, int(item->children_.size()), model_, item);
    model_->addRow(new_item, index);
    QModelIndex new_index = model_->index(int(item->children_.size() - 1), 0, index);
    ui->dialogView->setCurrentIndex(new_index);
    loadItem(new_item);
    ui->dialogTextEdit->setFocus();
    setModified(true);
}

void DialogView::onDialogSave()
{
    QFileInfo fileInfo(path_);
    auto ext = fileInfo.completeSuffix();
    if (0 == ext.compare("dlg", Qt::CaseInsensitive)) {
        nw::GffBuilder oa = nw::serialize(model_->dialog());
        oa.write_to(path_.toStdString());
        setModified(false);
    } else if (0 == ext.compare("dlg.json", Qt::CaseInsensitive)) {
        nlohmann::json j;
        nw::serialize(j, *model_->dialog());
        std::ofstream f{path_.toStdString()};
        f << std::setw(4) << j;
        setModified(false);
    }
}

void DialogView::onDialogSaveAs()
{
    auto fn = QFileDialog::getSaveFileName(this, "Save As..", path_, "Dlg (*.dlg *.dlg.json)");
    if (fn.isEmpty()) { return; }
    path_ = fn;

    QFileInfo fileInfo(path_);
    auto ext = fileInfo.completeSuffix();
    if (0 == ext.compare("dlg", Qt::CaseInsensitive)) {
        nw::GffBuilder oa = nw::serialize(model_->dialog());
        oa.write_to(path_.toStdString());
        setModified(false);
    } else if (0 == ext.compare("dlg.json", Qt::CaseInsensitive)) {
        nlohmann::json j;
        nw::serialize(j, *model_->dialog());
        std::ofstream f{path_.toStdString()};
        f << std::setw(4) << j;
        setModified(false);
    }
}

void DialogView::onDialogTextChanged()
{
    if (!current_item_) { return; }
    auto index = ui->dialogView->currentIndex();
    if (!index.isValid()) { return; }
    auto item = reinterpret_cast<DialogItem*>(index.internalPointer());
    item->ptr_->node->text.add(lang_, ui->dialogTextEdit->toPlainText().toStdString(), feminine_);
    emit model_->dataChanged(index, index);
    setModified(true);
}

void DialogView::onEndScriptChanged(const QString& value)
{
    if (!current_item_) { return; }
    current_item_->ptr_->parent->script_end = nw::Resref{value.toStdString()};
    setModified(true);
}

void DialogView::onLanguageChanged(nw::LanguageID lang, bool feminine)
{
    lang_ = lang;
    feminine_ = feminine;
    ui->dialogView->setFocus(); // Need to set focus or model will not update until focus is taken.
    model_->setLanguage(lang, feminine);
}

void DialogView::onNoZoomChanged(int state)
{
    model_->dialog()->prevent_zoom = !!state;
    setModified(true);
}

void DialogView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);
    auto indices = selected.indexes();
    if (indices.empty()) { return; }
    auto new_ptr = reinterpret_cast<DialogItem*>(indices[0].internalPointer());
    if (!new_ptr || new_ptr == current_item_) { return; }
    loadItem(new_ptr);
}

void DialogView::onSpeakersListAddClicked()
{
    if (!ui->speakersTable->isEnabled()) { return; }

    ui->speakersTable->insertRow(ui->speakersTable->rowCount());
    ui->speakersTable->setFocus();
    ui->speakersTable->setCurrentCell(ui->speakersTable->rowCount() - 1, 0);
    auto item = ui->speakersTable->item(ui->speakersTable->rowCount() - 1, 0);
    ui->speakersTable->editItem(item);
    setModified(true);
}

void DialogView::onSpeakerTableCellChanged(int row, int col)
{
    if (!current_item_) { return; }

    auto item = ui->speakersTable->item(row, col);
    if (row == 0) {
        item->setData(Qt::DisplayRole, "<owner>");
    } else {
        auto variant = item->data(Qt::DisplayRole);
        if (!variant.isNull()) {
            current_item_->ptr_->node->speaker = variant.toString().toStdString();
            setModified(true);
        }
    }
}

void DialogView::onSpeakersListDelClicked()
{
    if (!ui->speakersTable->isEnabled()) { return; }

    QModelIndexList selection = ui->speakersTable->selectionModel()->selectedRows();
    for (int i = 0; i < selection.count(); i++) {
        if (selection[i].row() == 0) { continue; }
        ui->speakersTable->removeRow(selection[i].row());
        setModified(true);
    }
}
