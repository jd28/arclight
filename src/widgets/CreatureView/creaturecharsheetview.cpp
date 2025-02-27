#include "creaturecharsheetview.h"
#include "ui_creaturecharsheetview.h"

#include "../../services/toolset/toolsetservice.h"
#include "../util/itemmodels.h"
#include "../util/strings.h"
#include "creatureview.h"

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include "nw/kernel/Resources.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Creature.hpp"
#include "nw/profiles/nwn1/scriptapi.hpp"

#include <QStandardItemModel>

// == CreaturePackageFilter ===================================================
// ============================================================================

CreatureClassFilter::CreatureClassFilter(nw::Creature* obj, int slot, QObject* parent)
    : QSortFilterProxyModel(parent)
    , obj_{obj}
    , slot_{slot}
{
}

bool CreatureClassFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    auto idx = sourceModel()->index(source_row, 0, source_parent);
    int class_id = sourceModel()->data(idx, Qt::UserRole + 1).toInt();
    const auto is_valid = sourceModel()->data(idx, Qt::UserRole + 2);

    if (!is_valid.toBool()) { return false; }
    if (slot_ == 0) { return true; }

    // If it occupies some slot below, filter it out.
    auto cls = nw::Class::make(class_id);
    for (int i = slot_ - 1; i > 0; --i) {
        if (obj_->levels.entries[i].id == cls) { return false; }
    }

    return true;
}

// == CreaturePackageFilter ===================================================
// ============================================================================

CreaturePackageFilter::CreaturePackageFilter(nw::Creature* obj, QObject* parent)
    : QSortFilterProxyModel(parent)
    , obj_{obj}
{
}

bool CreaturePackageFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    auto idx = sourceModel()->index(source_row, 0, source_parent);
    int class_id = sourceModel()->data(idx, Qt::UserRole + 2).toInt();
    return obj_->levels.level_by_class(nw::Class::make(class_id)) > 0;
}

// == CreatureCharSheetView ===================================================
// ============================================================================

CreatureCharSheetView::CreatureCharSheetView(nw::Creature* obj, CreatureView* parent)
    : ArclightTab(parent)
    , ui(new Ui::CreatureCharSheetView)
{
    ui->setupUi(this);
    loadCreature(obj);
    loadPortrait(obj);
    obj_ = obj;

    connect(this, &CreatureCharSheetView::modificationChanged, parent, &CreatureView::onModified);
}

CreatureCharSheetView::~CreatureCharSheetView()
{
    delete ui;
}

void CreatureCharSheetView::loadCreature(nw::Creature* obj)
{
    obj_ = nullptr;

    ui->firstName->setLocString(obj->name_first);
    ui->lastName->setLocString(obj->name_last);
    ui->tag->setText(to_qstring(obj->common.tag.view()));
    ui->resref->setText(to_qstring(obj->common.resref.view()));
    ui->genderSelector->setCurrentIndex(int(obj->gender));

    int first_invalid = -1;
    for (size_t i = 0; i < nw::LevelStats::max_classes; ++i) {
        auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(i + 1));
        auto combobox = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(i + 1));
        auto del = ui->classesWidget->findChild<QPushButton*>(QString("classDeleteButton_%1").arg(i + 1));

        connect(combobox, &QComboBox::currentIndexChanged, this, &CreatureCharSheetView::onClassChanged);
        connect(spinbox, &QSpinBox::valueChanged, this, &CreatureCharSheetView::onClassLevelChanged);
        connect(del, &QPushButton::clicked, this, &CreatureCharSheetView::onClassDeleteButtonClicked);

        auto proxy = new CreatureClassFilter(obj, int(i), this);
        cls_filters_.append(proxy);
        proxy->setSourceModel(toolset().class_model.get());
        proxy->sort(0);
        combobox->setModel(proxy);

        if (obj->levels.entries[i].id != nw::Class::invalid()) {
            QModelIndex sidx = toolset().class_model->index(*obj->levels.entries[i].id, 0, {});
            QModelIndex pidx = proxy->mapFromSource(sidx);
            combobox->setCurrentIndex(pidx.row());
            spinbox->setValue(obj->levels.entries[i].level);

            if (i == nw::LevelStats::max_classes - 1 || obj->levels.entries[i + 1].id == nw::Class::invalid()) {
                del->setEnabled(true);
            } else {
                del->setEnabled(false);
            }
        } else {
            if (first_invalid == -1) { first_invalid = int(i); }
            combobox->setCurrentIndex(-1);
            combobox->setDisabled(int(i) > first_invalid);
            spinbox->setDisabled(i > 0);
        }
    }

    pkg_filter_ = new CreaturePackageFilter(obj, this);
    pkg_filter_->setSourceModel(toolset().packages_model.get());
    pkg_filter_->sort(0);
    ui->packages->setModel(pkg_filter_);
    int pkg_idx = ui->packages->findData(obj->starting_package, Qt::UserRole + 1);
    ui->packages->setCurrentIndex(pkg_idx);

    obj_ = obj;

    loadStatsAbilities();
}

void CreatureCharSheetView::loadPortrait(nw::Creature* obj)
{
    ui->portraitEdit->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_ellipsis));

    if (auto portraits_2da = nw::kernel::twodas().get("portraits")) {
        auto base = portraits_2da->get<std::string>(obj->appearance.portrait_id, "BaseResRef");
        if (base) {
            auto base_resref = fmt::format("po_{}m", *base);
            auto portrait = nw::kernel::resman().demand_in_order(nw::Resref(base_resref),
                {nw::ResourceType::dds, nw::ResourceType::tga});

            if (portrait.bytes.size()) {
                auto img = nw::Image(std::move(portrait));
                QImage qi(img.data(), img.width(), img.height(),
                    img.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
                if (qi.height() > 128 || qi.width() > 128) {
                    qi = qi.scaled(128, 128, Qt::KeepAspectRatio);
                }

                // These are pre-flipped
                if (img.is_bio_dds()) {
                    qi.mirror();
                }

                QRect rect(0, 0, 64, 100); // This is specific to medium portraits
                qi = qi.copy(rect);
                ui->labelPortraitImage->setPixmap(QPixmap::fromImage(qi));
                ui->portraitLineEdit->setText(to_qstring("po_" + *base));
            }
        }
    } else {
        LOG_F(ERROR, "Failed to load portraits.2da");
    }
}

void CreatureCharSheetView::onReloadStats()
{
    LOG_F(INFO, "Reloading stats...");
    ui->stats->clear();
    loadStatsAbilities();
}

void CreatureCharSheetView::loadStatsAbilities()
{

#define ADD_INT_STAT(name, value, grp)                             \
    do {                                                           \
        auto p = ui->stats->makeIntegerProperty(name, value, grp); \
        p->read_only = true;                                       \
    } while (0)

    Property* grp_abilities = ui->stats->makeGroup("Abilities");
    ADD_INT_STAT("Strength", nwn1::get_ability_score(obj_, nwn1::ability_strength), grp_abilities);
    ADD_INT_STAT("Dexterity", nwn1::get_ability_score(obj_, nwn1::ability_dexterity), grp_abilities);
    ADD_INT_STAT("Constituion", nwn1::get_ability_score(obj_, nwn1::ability_constitution), grp_abilities);
    ADD_INT_STAT("Intelligence", nwn1::get_ability_score(obj_, nwn1::ability_intelligence), grp_abilities);
    ADD_INT_STAT("Wisdom", nwn1::get_ability_score(obj_, nwn1::ability_wisdom), grp_abilities);
    ADD_INT_STAT("Charisma", nwn1::get_ability_score(obj_, nwn1::ability_charisma), grp_abilities);
    ui->stats->addProperty(grp_abilities);

    Property* grp_saves = ui->stats->makeGroup("Saves");
    ADD_INT_STAT("Fortitude", nwn1::saving_throw(obj_, nwn1::saving_throw_fort), grp_saves);
    ADD_INT_STAT("Reflex", nwn1::saving_throw(obj_, nwn1::saving_throw_reflex), grp_saves);
    ADD_INT_STAT("Will", nwn1::saving_throw(obj_, nwn1::saving_throw_will), grp_saves);
    ui->stats->addProperty(grp_saves);

    Property* grp_skills = ui->stats->makeGroup("Skills");
    int i = 0;
    for (const auto& skill : nw::kernel::rules().skills.entries) {
        if (skill.valid()) {
            ADD_INT_STAT(to_qstring(nw::kernel::strings().get(skill.name)),
                nwn1::get_skill_rank(obj_, nw::Skill::make(i)), grp_skills);
        }
        ++i;
    }

    std::sort(grp_skills->children.begin(), grp_skills->children.end(), [](auto lhs, auto rhs) {
        return lhs->name < rhs->name;
    });
    ui->stats->addProperty(grp_skills);

#undef ADD_INT_STAT
}

// == Private Slots ===========================================================
// ============================================================================

void CreatureCharSheetView::onClassChanged(int index)
{
    if (!obj_ || index < 0) { return; }

    auto combobox = qobject_cast<QComboBox*>(sender());
    auto combobox_id = combobox->objectName().back().digitValue();

    auto class_slot = static_cast<size_t>(combobox_id - 1);
    auto class_id = nw::Class::make(combobox->currentData(Qt::UserRole + 1).toInt());
    bool new_class = obj_->levels.entries[class_slot].id == nw::Class::invalid();
    obj_->levels.entries[class_slot].id = class_id;

    auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(combobox_id));
    auto del = ui->classesWidget->findChild<QPushButton*>(QString("classDeleteButton_%1").arg(combobox_id));

    if (new_class) {
        if (class_slot > 0) {
            auto last_del = ui->classesWidget->findChild<QPushButton*>(QString("classDeleteButton_%1").arg(combobox_id - 1));
            last_del->setEnabled(false);
        }
        del->setEnabled(true);
        spinbox->setDisabled(false);
        spinbox->setValue(1);
        spinbox->setMinimum(1);

        if (combobox_id < 8) {
            auto combo = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(combobox_id + 1));
            combo->setEnabled(true);
        }

        emit classAdded(class_id);
    }
    pkg_filter_->invalidate();
    foreach (auto it, cls_filters_) {
        it->invalidate();
    }
    onReloadStats();
}

void CreatureCharSheetView::onClassDeleteButtonClicked()
{
    auto widget = qobject_cast<QPushButton*>(sender());
    auto widget_id = widget->objectName().back().digitValue();
    widget->setEnabled(false);

    int class_slot = widget_id - 1;
    if (class_slot <= 0) { return; }
    auto last_class_id = obj_->levels.entries[class_slot].id;

    obj_->levels.entries[class_slot].id = nw::Class::invalid();
    obj_->levels.entries[class_slot].level = 0;

    auto combo = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(widget_id));
    combo->setCurrentIndex(-1);

    auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(widget_id));
    spinbox->setMinimum(0);
    spinbox->setValue(0);
    spinbox->setEnabled(false);

    if (widget_id > 2) { // Never have the first button enabled...
        auto del = ui->classesWidget->findChild<QPushButton*>(QString("classDeleteButton_%1").arg(widget_id - 1));
        del->setEnabled(true);
    }

    if (widget_id < 8) {
        auto next_combo = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(widget_id + 1));
        next_combo->setEnabled(false);
    }

    // [TODO] Clear all spells or let the spell editor handle that..
    // auto sb = obj_->levels.spells(last_class_id);

    emit classRemoved(last_class_id);
    pkg_filter_->invalidate();
    foreach (auto it, cls_filters_) {
        it->invalidate();
    }
    onReloadStats();
}

void CreatureCharSheetView::onClassLevelChanged(int value)
{
    if (!obj_) { return; }

    auto spinbox = qobject_cast<QSpinBox*>(sender());
    auto class_slot = spinbox->objectName().back().digitValue() - 1;
    obj_->levels.entries[class_slot].level = int16_t(value);
    onReloadStats();
}
