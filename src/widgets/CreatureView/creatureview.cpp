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
    ui->openGLWidget->setCreature(obj);

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
}

CreatureView::~CreatureView()
{
    delete ui;
    if (owned_ && obj_) {
        nw::kernel::objects().destroy(obj_->handle());
    }
}

void CreatureView::onModified()
{
    // setModified(true);
    ui->openGLWidget->onDataChanged();
}
