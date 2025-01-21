#include "creaturespellselector.h"
#include "ui_creaturespellselector.h"

#include "checkboxdelegate.h"
#include "spinboxdelegate.h"
#include "textboxdialog.h"
#include "util/strings.h"

#include <nw/kernel/Rules.hpp>
#include <nw/kernel/Strings.hpp>
#include <nw/profiles/nwn1/scriptapi.hpp>
#include <nw/scriptapi.hpp>

extern "C" {
#include "fzy/match.h"
}

#include <QCheckBox>
#include <QDialog>
#include <QMap>
#include <QPair>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>


inline QString metamagic_to_name(nw::MetaMagicFlag meta)
{
    auto meta_info = nw::kernel::rules().metamagic.get(nwn1::metamagic_flag_to_idx(meta));
    if (!meta_info) { return {}; }
    return to_qstring(nw::kernel::strings().get(meta_info->name));
}

inline QString generate_memorized_summary(nw::Creature* obj, nw::Class class_)
{
    auto spellbook = obj->levels.spells(class_);
    if (!spellbook) { return {}; }

    QString result;
    for (size_t i = 0; i < nw::kernel::rules().maximum_spell_levels(); ++i) {
        result.append(QString("Level %1: (%2 remaining)\n")
                          .arg(i)
                          .arg(nwn1::get_available_spell_slots(obj, class_, int(i))));

        QMap<QPair<nw::Spell, nw::MetaMagicFlag>, int> spell_map;
        for (const auto& it : spellbook->memorized_[i]) {
            ++spell_map[{it.spell, it.meta}];
        }

        for (auto it = spell_map.begin(); it != spell_map.end(); ++it) {
            const QPair<nw::Spell, nw::MetaMagicFlag>& key = it.key();
            int count = it.value();

            auto sp_info = nw::kernel::rules().spells.get(key.first);
            if (!sp_info) { continue; }

            if (key.second != nw::metamagic_none) {
                result.append(QString("   %1 x %2 [%3]\n")
                                  .arg(count)
                                  .arg(to_qstring(nw::kernel::strings().get(sp_info->name)))
                                  .arg(key.second != nw::metamagic_none ? metamagic_to_name(key.second) : ""));
            } else {
                result.append(QString("   %1 x %2\n")
                                  .arg(count)
                                  .arg(to_qstring(nw::kernel::strings().get(sp_info->name))));
            }
        }
    }
    return result;
}

inline QString generate_known_summary(nw::Creature* obj, nw::Class class_)
{
    auto spellbook = obj->levels.spells(class_);
    if (!spellbook) { return {}; }

    QString result;
    for (size_t i = 0; i < nw::kernel::rules().maximum_spell_levels(); ++i) {
        int available = int(nwn1::compute_total_spells_knowable(obj, class_, int(i)) - spellbook->get_known_spell_count(i));
        result.append(QString("Level %1: (%2 remaining)\n")
                          .arg(i)
                          .arg(available));

        for (const auto& it : spellbook->known_[i]) {
            auto sp_info = nw::kernel::rules().spells.get(it);
            if (!sp_info) { continue; }
            result.append(QString("   %1\n")
                              .arg(to_qstring(nw::kernel::strings().get(sp_info->name))));
        }
    }
    return result;
}

// == CreatureSpellModel ======================================================
// ============================================================================

CreatureSpellModel::CreatureSpellModel(nw::Creature* creature, nw::Class cls, QObject* parent)
    : QAbstractTableModel(parent)
    , creature_{creature}
    , class_{cls}
{
}

int CreatureSpellModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) { return 0; }
    return static_cast<int>(nw::kernel::rules().spells.entries.size());
}

int CreatureSpellModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) { return 0; }
    return 3;
}

QVariant CreatureSpellModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }

    if (role == Qt::TextAlignmentRole) {
        return index.column() > 0 ? Qt::AlignCenter : QVariant();
    }

    if (role == Qt::DisplayRole) {
        auto spell = nw::Spell::make(index.row());
        auto cls = nw::kernel::rules().classes.get(class_);
        if (!cls) { return {}; }

        auto sp = nw::kernel::rules().spells.get(spell);
        if (!sp || !sp->valid()) { return {}; }

        auto spellbook = creature_->levels.spells(class_);
        if (!spellbook) { return {}; }

        switch (index.column()) {
        default:
            return {};
        case 0:
            return QString::fromUtf8(nw::kernel::strings().get(sp->name));
        case 1: {
            int base = nw::kernel::rules().classes.get_spell_level(class_, spell);
            if (metamagic_ == nw::metamagic_none) {
                return base;
            } else {
                auto meta_info = nw::kernel::rules().metamagic.get(nwn1::metamagic_flag_to_idx(metamagic_));
                return meta_info ? base + meta_info->level_adjustment : base;
            }
        } break;
        case 2:
            if (cls->memorizes_spells) {
                return nwn1::get_available_spell_uses(creature_, class_, spell, 0, metamagic_);
            } else {
                return nwn1::get_knows_spell(creature_, class_, spell);
            }
        }
    } else if (role == Qt::UserRole + 1) {
        return *class_;
    } else if (role == Qt::UserRole + 2) {
        return *metamagic_;
    }

    return {};
}

QVariant CreatureSpellModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    auto cls = nw::kernel::rules().classes.get(class_);
    if (!cls) { return {}; }

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        default:
            return {};
        case 0:
            return "Spell";
        case 1:
            return "Level";
        case 2:
            return cls->memorizes_spells ? "Uses" : "Known";
        }
    }
    return {};
}

Qt::ItemFlags CreatureSpellModel::flags(const QModelIndex& index) const
{
    if (index.column() == 2) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
    return QAbstractTableModel::flags(index);
}

bool CreatureSpellModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole || index.column() != 2)
        return false;

    auto cls = nw::kernel::rules().classes.get(class_);
    if (!cls) { return false; }

    auto spell = nw::Spell::make(index.row());
    auto sp_info = nw::kernel::rules().spells.get(spell);
    if (!sp_info) { return false; }

    auto spellbook = creature_->levels.spells(class_);
    int spell_level = nw::kernel::rules().classes.get_spell_level(class_, spell);
    if (cls->memorizes_spells) {
        if (auto meta_info = nw::kernel::rules().metamagic.get(nwn1::metamagic_flag_to_idx(metamagic_))) {
            spell_level += meta_info->level_adjustment;
        }

        int newval = value.toInt();
        int current = nwn1::get_available_spell_uses(creature_, class_, spell, 0, metamagic_);

        if (newval > current) {
            for (int i = 0; i < newval - current; ++i) {
                if (!nwn1::add_memorized_spell(creature_, class_, spell, metamagic_)) {
                    break;
                }
            }
        } else if (newval < current) {
            for (int i = 0; i < current - newval; ++i) {
                int slot = spellbook->find_memorized_slot(spell_level, spell, metamagic_);
                if (slot == -1) { break; }
                spellbook->clear_memorized_spell_slot(spell_level, slot);
            }
        }
    } else {
        bool add = value.toBool();
        if (add) {
            return spellbook->add_known_spell(spell_level, spell);
        } else {
            spellbook->remove_known_spell(spell_level, spell);
        }
    }

    return true;
}

int CreatureSpellModel::getRemainingSlots(int currentRow) const
{
    if (currentRow < 0) { return 0; }

    auto spell = nw::Spell::make(currentRow);

    auto cls = nw::kernel::rules().classes.get(class_);
    if (!cls) { return 0; }

    auto sp = nw::kernel::rules().spells.get(spell);
    if (!sp) { return 0; }

    auto spell_level = nw::kernel::rules().classes.get_spell_level(class_, spell);
    if (spell_level == -1) { return 0; }

    if (metamagic_ != nw::metamagic_none) {
        auto meta = nw::kernel::rules().metamagic.get(nwn1::metamagic_flag_to_idx(metamagic_));
        if (!meta) { return 0; }
        spell_level += meta->level_adjustment;
    }
    return nwn1::get_available_spell_slots(creature_, class_, spell_level);
}

void CreatureSpellModel::setClass(nw::Class cls)
{
    class_ = cls;
}

void CreatureSpellModel::setMetaMagic(nw::MetaMagicFlag meta)
{
    if (meta == nw::metamagic_any) { meta = nw::metamagic_none; }
    metamagic_ = meta;
}

// == CreatureSpellSortFilterProxyModel =======================================
// ============================================================================

CreatureSpellSortFilterProxyModel::CreatureSpellSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

void CreatureSpellSortFilterProxyModel::setFilterSpellName(const QString& name)
{
    name_ = name;
    invalidateFilter();
}

void CreatureSpellSortFilterProxyModel::setFilterSpellLevel(int level)
{
    level_ = level;
    invalidateFilter();
}

bool CreatureSpellSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex name_idx = sourceModel()->index(sourceRow, 0, sourceParent);
    QModelIndex level_idx = sourceModel()->index(sourceRow, 1, sourceParent);

    QString spell_name = sourceModel()->data(name_idx).toString();
    auto class_ = nw::Class::make(sourceModel()->data(level_idx, Qt::UserRole + 1).toInt());
    auto spell = nw::Spell::make(sourceRow);
    auto meta = nw::MetaMagicFlag::make(sourceModel()->data(level_idx, Qt::UserRole + 2).toInt());

    int spell_level = nw::kernel::rules().classes.get_spell_level(class_, spell);
    if (spell_level == -1) { return false; }

    if (meta != nw::metamagic_none) {
        if (auto meta_info = nw::kernel::rules().metamagic.get(nwn1::metamagic_flag_to_idx(meta))) {
            spell_level += meta_info->level_adjustment;
        }
    }

    if (spell_level >= int(nw::kernel::rules().maximum_spell_levels())) {
        return false;
    }

    bool name_match = name_.isEmpty() || has_match(name_.toStdString().c_str(), spell_name.toStdString().c_str());
    bool level_match = level_ == -1 || spell_level == level_;

    return name_match && level_match;
}

// == CreatureSpellSelector ===================================================
// ============================================================================

inline void memorized_delegate_config(QSpinBox* spinBox, const QModelIndex& index)
{
    const QSortFilterProxyModel* proxy = static_cast<const QSortFilterProxyModel*>(index.model());
    const CreatureSpellModel* model = static_cast<const CreatureSpellModel*>(proxy->sourceModel());

    auto class_ = nw::Class::make(index.data(Qt::UserRole + 1).toInt());
    auto cls = nw::kernel::rules().classes.get(class_);
    if (cls) {
        QVariant value = index.data(Qt::DisplayRole);

        int sourceRow = proxy->mapToSource(index).row();
        int remaining = model->getRemainingSlots(sourceRow);
        int current = value.toInt();

        spinBox->setMinimum(0);
        spinBox->setMaximum(remaining + current);
        if (remaining == 0 && current == 0) {
            spinBox->setEnabled(false);
        }
    }
};

CreatureSpellSelector::CreatureSpellSelector(nw::Creature* creature, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CreatureSpellSelector)
    , creature_{creature}
{
    ui->setupUi(this);

    for (const auto& classes : creature_->levels.entries) {
        if (classes.id == nw::Class::invalid()) { break; }
        auto cls = nw::kernel::rules().classes.get(classes.id);
        if (!cls->spellcaster) { continue; }

        ui->classes->addItem(to_qstring(nw::kernel::strings().get(cls->name)), *classes.id);

        if (class_ == nw::Class::invalid()) {
            class_ = classes.id;
            ui->classes->setCurrentIndex(ui->classes->count() - 1);
        }
    }

    uint32_t i = 0;
    for (const auto& meta : nw::kernel::rules().metamagic.entries) {
        if (i == 0 || nw::knows_feat(creature_, meta.feat)) {
            ui->metamagic->addItem(to_qstring(nw::kernel::strings().get(meta.name)), i);
        }
        ++i;
    }

    // Set up model and proxy
    model_ = new CreatureSpellModel(creature_, class_, this);
    proxy_ = new CreatureSpellSortFilterProxyModel(this);
    proxy_->setSourceModel(model_);
    ui->spells->setModel(proxy_);
    ui->spells->model()->sort(0);
    ui->spells->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->spells->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->spells->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    ui->level->addItem("All");
    for (size_t j = 0; j < nw::kernel::rules().maximum_spell_levels(); ++j) {
        ui->level->addItem(QString::number(j));
    }

    auto cls = nw::kernel::rules().classes.get(class_);
    ui->classes->setEnabled(!!cls);
    ui->level->setEnabled(!!cls);
    ui->metamagic->setEnabled(!!cls && cls->memorizes_spells);

    memorize_delegate_ = new SpinBoxDelegate(memorized_delegate_config, this);
    known_delegate_ = new CheckBoxDelegate(this);
    if (cls->memorizes_spells) {
        ui->spells->setItemDelegateForColumn(2, memorize_delegate_);
        ui->spells->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    } else {
        ui->spells->setItemDelegateForColumn(2, known_delegate_);
        ui->spells->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    connect(ui->classes, &QComboBox::currentIndexChanged, this, &CreatureSpellSelector::onClassChanged);
    connect(ui->clear, &QPushButton::clicked, this, &CreatureSpellSelector::onClearClicked);
    connect(ui->level, &QComboBox::currentIndexChanged, this, &CreatureSpellSelector::onSpellLevelChanged);
    connect(ui->metamagic, &QComboBox::currentIndexChanged, this, &CreatureSpellSelector::onMetaMagicChanged);
    connect(ui->filter, &QLineEdit::textChanged, this, &::CreatureSpellSelector::onFilterChanged);
    connect(ui->summary, &QPushButton::clicked, this, &CreatureSpellSelector::onSummaryClicked);
}

CreatureSpellSelector::~CreatureSpellSelector()
{
    delete ui;
}

void CreatureSpellSelector::onClassChanged(int currentIndex)
{
    Q_UNUSED(currentIndex);
    class_ = nw::Class::make(ui->classes->currentData().toInt());

    auto cls = nw::kernel::rules().classes.get(class_);
    if (!cls) { return; }

    model_->setClass(class_);

    // Reset the other comboboxes
    ui->level->setCurrentIndex(0);
    if (ui->metamagic->isEnabled() && !cls->memorizes_spells) {
        ui->metamagic->setCurrentIndex(0);
    }
    ui->metamagic->setEnabled(cls->memorizes_spells);

    if (cls->memorizes_spells) {
        ui->spells->setItemDelegateForColumn(2, memorize_delegate_);
        ui->spells->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    } else {
        ui->spells->setItemDelegateForColumn(2, known_delegate_);
        ui->spells->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    ui->spells->reset();
    ui->spells->scrollToTop();
    proxy_->invalidate();
}

void CreatureSpellSelector::onClearClicked()
{
    auto cls = nw::kernel::rules().classes.get(class_);
    if (!cls) { return; }

    auto spellbook = creature_->levels.spells(class_);
    auto spell_level = ui->level->currentIndex() - 1;

    int i = 0;
    if (cls->memorizes_spells) {
        for (auto& it : spellbook->memorized_) {
            if (spell_level == -1 || spell_level == i) {
                for (auto& entry : it) {
                    entry = nw::SpellEntry{};
                }
            }
            ++i;
        }
    } else {
        for (auto& it : spellbook->known_) {
            if (spell_level == -1 || spell_level == i) {
                it.clear();
            }
            ++i;
        }
    }

    auto topLeft = model_->index(0, 2);
    auto bottomRight = model_->index(model_->rowCount(), 2);
    emit model_->dataChanged(topLeft, bottomRight);
    ui->spells->viewport()->update();
}

void CreatureSpellSelector::onFilterChanged(const QString& text)
{
    proxy_->setFilterSpellName(text);
}

void CreatureSpellSelector::onMetaMagicChanged(int currentIndex)
{
    if (currentIndex == 0) {
        metamagic_ = nw::metamagic_none;
    } else {
        metamagic_ = nwn1::metamagic_idx_to_flag(nw::MetaMagic::make(currentIndex));
    }
    model_->setMetaMagic(metamagic_);
    ui->spells->reset();
    ui->spells->scrollToTop();
    proxy_->invalidate();
}

void CreatureSpellSelector::onSpellLevelChanged(int currentIndex)
{
    proxy_->setFilterSpellLevel(currentIndex - 1);
}

void CreatureSpellSelector::onSummaryClicked()
{
    auto cls = nw::kernel::rules().classes.get(class_);
    if (!cls) { return; }

    QString text;
    if (cls->memorizes_spells) {
        text = generate_memorized_summary(creature_, class_);
    } else {
        text = generate_known_summary(creature_, class_);
    }
    TextBoxDialog dialog(text, 400, 500);
    dialog.exec();
}
