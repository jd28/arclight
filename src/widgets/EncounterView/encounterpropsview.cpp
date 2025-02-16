#include "encounterpropsview.h"
#include "ui_encounterpropsview.h"

#include "encountercreaturemodel.h"

#include "../../services/toolsetservice.h"
#include "../checkboxdelegate.h"
#include "../util/itemmodels.h"
#include "../util/strings.h"

#include "nw/objects/Encounter.hpp"

#include <QMouseEvent>
#include <QStandardItemModel>
#include <QStringListModel>

EncounterPropsView::EncounterPropsView(nw::Encounter* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::EncounterPropsView)
    , obj_{obj}
{
    if (!obj_) { return; }
    ui->setupUi(this);

    ui->name->setLocString(obj_->common.name);
    ui->tag->setText(to_qstring(obj_->tag().view()));
    ui->resref->setText(to_qstring(obj_->common.resref.view()));

    loadProperties();

    creatures_ = new EncounterCreatureModel(obj_, undoStack(), this);
    ui->creatures->setModel(creatures_);
    ui->creatures->setItemDelegateForColumn(4, new CheckBoxDelegate(this));

    ui->creatures->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->creatures->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->creatures->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->creatures->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->creatures->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    ui->creatures->viewport()->installEventFilter(this);
    ui->creatures->installEventFilter(this);
}

EncounterPropsView::~EncounterPropsView()
{
    delete ui;
}

void EncounterPropsView::loadProperties()
{
    ui->properties->setUndoStack(undoStack());

    Property* prop = nullptr;

    prop = ui->properties->makeBoolProperty("Active", obj_->active);
    prop->on_set = [this](const QVariant& value) {
        obj_->active = value.toBool();
    };
    ui->properties->addProperty(prop);

    int row = findStandardItemIndex(toolset().faction_model.get(), obj_->faction);
    prop = ui->properties->makeEnumProperty("Faction", row, toolset().faction_model.get());
    prop->on_set = [this](const QVariant& value) {
        auto idx = toolset().faction_model->index(value.toInt(), 0);
        obj_->faction = static_cast<uint16_t>(idx.data(Qt::UserRole + 1).toInt());
    };
    ui->properties->addProperty(prop);

    prop = ui->properties->makeIntegerProperty("Maximum Creatures", obj_->creatures_max);
    prop->int_config.min = 1;
    prop->int_config.max = 8;
    prop->on_set = [this](const QVariant& value) {
        obj_->creatures_max = value.toInt();
    };
    ui->properties->addProperty(prop);

    prop = ui->properties->makeIntegerProperty("Minimum Creatures", obj_->creatures_recommended);
    prop->int_config.min = 1;
    prop->int_config.max = 8;
    prop->on_set = [this](const QVariant& value) {
        obj_->creatures_recommended = value.toInt();
    };
    ui->properties->addProperty(prop);

    prop = ui->properties->makeBoolProperty("Player Only", obj_->player_only);
    prop->on_set = [this](const QVariant& value) {
        obj_->player_only = value.toBool();
    };
    ui->properties->addProperty(prop);

    prop = ui->properties->makeBoolProperty("Respawns", obj_->reset);
    prop->on_set = [this](const QVariant& value) {
        obj_->reset = value.toBool();
        // Enable / disable other crap.
    };
    ui->properties->addProperty(prop);

    respawn_count_ = ui->properties->makeIntegerProperty("Respawn Count", obj_->respawns);
    respawn_count_->int_config.min = -1;
    respawn_count_->int_config.max = 32000;
    respawn_count_->on_set = [this](const QVariant& value) {
        obj_->respawns = value.toInt();
    };
    ui->properties->addProperty(respawn_count_);

    prop = ui->properties->makeIntegerProperty("Respawn Time (seconds)", obj_->reset_time);
    prop->int_config.min = 1;
    prop->int_config.max = 32000;
    prop->on_set = [this](const QVariant& value) {
        obj_->reset_time = value.toInt();
    };
    ui->properties->addProperty(prop);

    prop = ui->properties->makeBoolProperty("Respawns Infinitely", obj_->respawns == -1);
    prop->on_set = [this](const QVariant& value) {
        if (value.toBool()) {
            obj_->respawns = -1;
        }
        ui->properties->model()->updateReadOnly(respawn_count_);
    };
    ui->properties->addProperty(prop);

    auto spawn_model = new QStringListModel(this);
    spawn_model->setStringList(QStringList() << "Continuous" << "Single Shot");
    prop = ui->properties->makeEnumProperty("Spawn Option", obj_->spawn_option, spawn_model);
    prop->on_set = [this](const QVariant& value) {
        obj_->spawn_option = value.toInt();
    };
    ui->properties->addProperty(prop);

#define ADD_SCRIPT(name, resref)                                                                                 \
    do {                                                                                                         \
        auto p = ui->properties->makeStringProperty(name, to_qstring(obj_->scripts.resref.view()), grp_scripts); \
        p->on_set = [this](const QVariant& value) {                                                              \
            obj_->scripts.resref = nw::Resref{value.toString().toStdString()};                                   \
        };                                                                                                       \
    } while (0)

    Property* grp_scripts = ui->properties->makeGroup("Scripts");
    ADD_SCRIPT("On Entered", on_entered);
    ADD_SCRIPT("On Exhausted", on_exhausted);
    ADD_SCRIPT("On Exit", on_exit);
    ADD_SCRIPT("On Heartbeat", on_heartbeat);
    ADD_SCRIPT("On User Defined", on_user_defined);
    ui->properties->addProperty(grp_scripts);

#undef ADD_SCRIPT
}

bool EncounterPropsView::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = ui->creatures->indexAt(mouseEvent->pos());
        if (index.isValid() && index.column() == 4) {
            bool currentValue = index.data(Qt::DisplayRole).toBool();
            ui->creatures->model()->setData(index, !currentValue);
            return true;
        }
    } else if (event->type() == QEvent::KeyPress) {
        LOG_F(INFO, "Key press event");
        auto ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Delete) {
            QModelIndex index = ui->creatures->currentIndex();
            if (index.isValid()) {
                undoStack()->push(new RemoveEncounterCreatureCommand(obj_->creatures[index.row()],
                    index.row(), static_cast<EncounterCreatureModel*>(ui->creatures->model())));
                return true;
            }
        }
    }

    return ArclightTab::eventFilter(object, event);
}
