#include "creatureview.h"
#include "ui_creatureview.h"

#include "../VariableTableView/variabletableview.h"
#include "../strreftextedit.h"
#include "../util/strings.h"

#include "creatureabilitiesselector.h"
#include "creatureappearanceview.h"
#include "creaturecharsheetview.h"
#include "creatureequipview.h"
#include "creaturefeatselector.h"
#include "creatureinventorypanel.h"
#include "creaturepropertiestab.h"
#include "creaturepropertiesview.h"
#include "creaturespellselector.h"
#include "creaturestatsview.h"

#include "nw/kernel/Objects.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Creature.hpp"

#include <QApplication>
#include <QScreen>
#include <QTextEdit>

CreatureView::CreatureView(nw::Resource res, QWidget* parent)
    : CreatureView(nw::kernel::objects().load<nw::Creature>(res.resref), parent)
{
    owned_ = true;
}

CreatureView::CreatureView(nw::Creature* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::CreatureView)
{
    if (!obj) { return; }

    ui->setupUi(this);

    auto width = qApp->primaryScreen()->geometry().width();
    ui->splitter->setSizes(QList<int>() << int(width * 0.5) << int(width * 0.5));

    auto charsheet = new CreatureCharSheetView(obj, this);
    charsheet->setEnabled(!readOnly());
    ui->tabWidget->addTab(charsheet, "Sheet");
    addTab(charsheet);

    auto props = new CreaturePropertiesTab(obj, this);
    props->setEnabled(!readOnly());
    ui->tabWidget->addTab(props, "Properties");
    addTab(props);
    connect(props->properties(), &CreaturePropertiesView::reloadStats, charsheet, &CreatureCharSheetView::onReloadStats);

    auto appearance = new CreatureAppearanceView(obj, this);
    appearance->setEnabled(!readOnly());
    ui->tabWidget->addTab(appearance, "Appearance");
    addTab(appearance);
    connect(appearance, &CreatureAppearanceView::updateModel, this, &CreatureView::onUpdateModel);

    auto inv = new CreatureInventoryPanel(this);
    inv->setEnabled(!readOnly());
    inv->setCreature(obj);
    ui->tabWidget->addTab(inv, "Inventory");
    addTab(inv);

    auto feats = new CreatureFeatSelector(obj, this);
    ui->tabWidget->addTab(feats, "Feats");
    addTab(feats);
    connect(feats->model(), &CreatureFeatSelectorModel::featsChanged, charsheet, &CreatureCharSheetView::onReloadStats);

    auto spells = new CreatureSpellSelector(obj, this);
    ui->tabWidget->addTab(spells, "Spells");
    addTab(spells);

    auto abilities = new CreatureAbilitiesSelector(obj, this);
    ui->tabWidget->addTab(abilities, "Special Abilities");
    addTab(abilities);

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
    addTab(variables);

    auto description = new StrrefTextEdit(this);
    description->setLocstring(obj->description);
    ui->tabWidget->addTab(description, "Description");

    auto comments = new QTextEdit(this);
    comments->setText(to_qstring(obj->common.comment));
    ui->tabWidget->addTab(comments, "Comments");

    obj_ = obj;
    onUpdateModel();
}

CreatureView::~CreatureView()
{
    delete ui;
    if (owned_ && obj_) {
        nw::kernel::objects().destroy(obj_->handle());
    }
}

void CreatureView::onUpdateModel()
{
    auto appearances_2da = nw::kernel::twodas().get("appearance");
    std::string model_name;
    // [TODO] Can't do parts based models yet..
    if (!appearances_2da->get_to(*obj_->appearance.id, "RACE", model_name) || model_name.length() <= 1) {
        LOG_F(INFO, "Can't render model.");
    } else {
        nw::Resref resref{model_name};

        LOG_F(INFO, "Loading model: {}", resref.view());
        auto model = load_model(resref.view());
        if (!model) {
            LOG_F(ERROR, "Failed to load model: {}", resref.view());
            return;
        }

        if (!model->load_animation("pause1")) {
            model->load_animation("cpause1");
        }
        LOG_F(INFO, "Model loaded: {}, nodes={}", resref.view(), model->nodes_.size());
        ui->openGLWidget->setModel(std::move(model));
        LOG_F(INFO, "Model set on RenderWidget");
    }
}
