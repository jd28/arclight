#include "creatureview.h"
#include "ui_creatureview.h"

#include "../VariableTableView/variabletableview.h"
#include "../util/strings.h"

#include "creatureabilitiesselector.h"
#include "creatureappearanceview.h"
#include "creaturecharsheetview.h"
#include "creatureequipview.h"
#include "creaturefeatselector.h"
#include "creaturefeatselectormodel.h"
#include "creatureinventorypanel.h"
#include "creaturepropertiesview.h"
#include "creaturespellselector.h"
#include "creaturestatsview.h"

#include "nw/objects/Creature.hpp"

#include <QApplication>
#include <QScreen>
#include <QTextEdit>

CreatureView::CreatureView(nw::Creature* obj, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::CreatureView)
{
    ui->setupUi(this);
    ui->openGLWidget->setCreature(obj);

    auto width = qApp->primaryScreen()->geometry().width();
    ui->splitter->setSizes(QList<int>() << int(width * 1.8 / 8) << int(width * 3.7 / 8) << int(width * 2.5) / 8);

    auto charsheet = new CreatureCharSheetView(obj, this);
    charsheet->setEnabled(!readOnly());
    ui->tabWidget->addTab(charsheet, "Sheet");

    auto props = new CreaturePropertiesView(this);
    props->setEnabled(!readOnly());
    props->setCreature(obj);
    ui->tabWidget->addTab(props, "Properties");

    auto stats = new CreatureStatsView(obj, this);
    ui->tabWidget->addTab(stats, "Statistics");

    connect(props, &CreaturePropertiesView::updateStats, stats, &CreatureStatsView::updateAll);

    auto feats = new CreatureFeatSelector(obj, this);
    connect(feats->model(), &CreatureFeatSelectorModel::featsChanged, stats, &CreatureStatsView::updateAll);
    ui->tabWidget->addTab(feats, "Feats");

    auto spells = new CreatureSpellSelector(obj, this);
    // connect(feats->model(), &CreatureFeatSelectorModel::featsChanged, stats, &CreatureStatsView::updateAll);
    ui->tabWidget->addTab(spells, "Spells");

    auto abilities = new CreatureAbilitiesSelector(obj, this);
    ui->tabWidget->addTab(abilities, "Special Abilities");

    auto appearance = new CreatureAppearanceView(obj, this);
    appearance->setEnabled(!readOnly());
    ui->tabWidget->addTab(appearance, "Appearance");
    connect(appearance, &CreatureAppearanceView::dataChanged, this, &CreatureView::onModified);

    auto inv = new CreatureInventoryPanel(this);
    inv->setEnabled(!readOnly());
    inv->setCreature(obj);
    ui->tabWidget->addTab(inv, "Inventory");

    auto variables = new VariableTableView(this);
    variables->setEnabled(!readOnly());
    variables->setLocals(&obj->common.locals);
    ui->tabWidget->addTab(variables, tr("Variables"));
}

CreatureView::~CreatureView()
{
    delete ui;
}

void CreatureView::onModified()
{
    ui->openGLWidget->onDataChanged();
}
