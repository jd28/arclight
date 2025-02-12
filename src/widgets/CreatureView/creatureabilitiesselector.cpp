#include "creatureabilitiesselector.h"
#include "ui_creatureabilitiesselector.h"

#include "../ArclightView.h"
#include "../spinboxdelegate.h"
#include "../textboxdialog.h"
#include "../util/strings.h"

#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/objects/Creature.hpp"
#include "nw/profiles/nwn1/scriptapi.hpp"

extern "C" {
#include "fzy/match.h"
}

#include <QSpinBox>
#include <QUndoCommand>

class CreatureAbilitiesModel;

class SetSpecialAbilityLevelCommand : public QUndoCommand {
public:
    SetSpecialAbilityLevelCommand(CreatureAbilitiesModel* model, nw::Creature* obj,
        nw::Spell spell, int previous, int next)
        : model_(model)
        , obj_(obj)
        , spell_(spell)
        , previous_(previous)
        , next_(next)
        , row_(*spell)
    {
        auto sp = nw::kernel::rules().spells.get(spell);
        if (sp) {
            setText(QString("Set %1 level to %2")
                    .arg(QString::fromUtf8(nw::kernel::strings().get(sp->name)))
                    .arg(next_));
        }
    }

    void undo() override
    {
        nwn1::set_special_ability_level(obj_, spell_, previous_);
        notifyModel();
    }

    void redo() override
    {
        nwn1::set_special_ability_level(obj_, spell_, next_);
        notifyModel();
    }

private:
    void notifyModel()
    {
        if (model_) {
            QModelIndex index = model_->index(row_, 2);
            emit model_->dataChanged(index, index);
        }
    }

    CreatureAbilitiesModel* model_;
    nw::Creature* obj_;
    nw::Spell spell_;
    int previous_;
    int next_;
    int row_;
};

class SetSpecialAbilityUsesCommand : public QUndoCommand {
public:
    SetSpecialAbilityUsesCommand(CreatureAbilitiesModel* model, nw::Creature* obj,
        nw::Spell spell, int previous, int next)
        : model_(model)
        , obj_(obj)
        , spell_(spell)
        , previous_(previous)
        , next_(next)
        , row_(*spell)
    {
    }

    void undo() override
    {
        nwn1::set_special_ability_uses(obj_, spell_, previous_);
        notifyModel();
    }

    void redo() override
    {
        nwn1::set_special_ability_uses(obj_, spell_, next_);
        notifyModel();
    }

private:
    void notifyModel()
    {
        if (model_) {
            QModelIndex index = model_->index(row_, 3);
            emit model_->dataChanged(index, index);
        }
    }

    CreatureAbilitiesModel* model_;
    nw::Creature* obj_;
    nw::Spell spell_;
    int previous_;
    int next_;
    int row_;
};

inline QString generate_abilities_summary(nw::Creature* obj)
{
    QString result{"Special Abilities\n"};

    QMap<nw::Spell, int> spell_map;
    for (const auto& it : obj->combat_info.special_abilities) {
        ++spell_map[{it.spell}];
    }

    for (auto it = spell_map.begin(); it != spell_map.end(); ++it) {
        int count = it.value();

        auto sp_info = nw::kernel::rules().spells.get(it.key());
        if (!sp_info) { continue; }

        result.append(QString("   %1 x %2\n")
                          .arg(count)
                          .arg(to_qstring(nw::kernel::strings().get(sp_info->name))));
    }
    return result;
}

// == CreatureAbilitiesModel ==================================================
// ============================================================================

CreatureAbilitiesModel::CreatureAbilitiesModel(nw::Creature* creature, QUndoStack* undo, QObject* parent)
    : QAbstractTableModel(parent)
    , creature_{creature}
    , undo_{undo}
{
}

int CreatureAbilitiesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) { return 0; }
    return static_cast<int>(nw::kernel::rules().spells.entries.size());
}

int CreatureAbilitiesModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) { return 0; }
    return 4;
}

QVariant CreatureAbilitiesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }

    if (role == Qt::TextAlignmentRole) {
        return index.column() > 1 ? Qt::AlignCenter : QVariant();
    }

    if (role == Qt::DisplayRole) {
        auto spell = nw::Spell::make(index.row());

        auto sp = nw::kernel::rules().spells.get(spell);
        if (!sp || !sp->valid()) { return {}; }

        switch (index.column()) {
        default:
            return {};
        case 0:
            return QString::fromUtf8(nw::kernel::strings().get(sp->name));
        case 1: {
            if (sp->user_type == 1) {
                return "Spells";
            } else if (sp->user_type == 2) {
                return "Monster Abilities";
            }
        } break;
        case 2:
            if (sp->user_type == 1) {
                return nwn1::get_special_ability_level(creature_, spell);
            }
            break;
        case 3:
            return nwn1::get_special_ability_uses(creature_, spell);
            break;
        }
    }

    return {};
}

QVariant CreatureAbilitiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        default:
            return {};
        case 0:
            return "Name";
        case 1:
            return "Type";
        case 2:
            return "Level";
        case 3:
            return "Uses";
        }
    }
    return {};
}

Qt::ItemFlags CreatureAbilitiesModel::flags(const QModelIndex& index) const
{
    if (index.column() >= 2) {
        auto spell = nw::Spell::make(index.row());
        auto sp = nw::kernel::rules().spells.get(spell);
        if (sp->user_type == 2 && index.column() == 2) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        } else {
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
        }
    }
    return QAbstractTableModel::flags(index);
}

bool CreatureAbilitiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole || index.column() < 2) { return false; }

    auto spell = nw::Spell::make(index.row());
    auto sp = nw::kernel::rules().spells.get(spell);
    if (!sp || !sp->valid()) { return false; }

    int next = value.toInt();

    if (index.column() == 2) {
        int previous = nwn1::get_special_ability_level(creature_, spell);
        if (previous != next) {
            auto* selector = qobject_cast<CreatureAbilitiesSelector*>(parent());
            if (selector) {
                selector->undoStack()->push(
                    new SetSpecialAbilityLevelCommand(this, creature_, spell, previous, next));
            }
            return true;
        }
    } else if (index.column() == 3) {
        int previous = nwn1::get_special_ability_uses(creature_, spell);
        if (previous != next) {
            undo_->push(new SetSpecialAbilityUsesCommand(this, creature_, spell, previous, next));
            return true;
        }
    }

    return false;
}

// == CreatureAbilitiesSortFilterProxyModel ===================================
// ============================================================================

CreatureAbilitiesSortFilterProxyModel::CreatureAbilitiesSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

void CreatureAbilitiesSortFilterProxyModel::setFilterSpellName(const QString& name)
{
    name_ = name;
    invalidateFilter();
}

bool CreatureAbilitiesSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex name_idx = sourceModel()->index(sourceRow, 0, sourceParent);
    QString spell_name = sourceModel()->data(name_idx).toString();

    auto spell = nw::Spell::make(sourceRow);
    auto sp_info = nw::kernel::rules().spells.get(spell);
    if (!sp_info || !sp_info->valid()) { return false; }
    if (sp_info->user_type != 1 && sp_info->user_type != 2) {
        return false;
    }

    bool name_match = name_.isEmpty() || has_match(name_.toStdString().c_str(), spell_name.toStdString().c_str());
    return name_match;
}

bool CreatureAbilitiesSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    // Make sure empty uses for monster abilities are always at the bottom when sorting
    // by that column.
    if (left.column() == 2 && right.column() == 2) {
        QVariant lhs = sourceModel()->data(left);
        QVariant rhs = sourceModel()->data(right);

        bool l = !lhs.isValid();
        bool r = !rhs.isValid();

        if (l && r) { return false; }

        if (l || r) {
            auto order = (sortColumn() == 2) ? sortOrder() : Qt::AscendingOrder;
            return (l && order == Qt::DescendingOrder) || (r && order == Qt::AscendingOrder);
        }

        return lhs.toInt() < rhs.toInt();
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

// == CreatureAbilitiesSelector ============================================
// ============================================================================

inline void level_delegate_config(QSpinBox* spinBox, const QModelIndex& index)
{
    Q_UNUSED(index);
    spinBox->setMinimum(0);
    spinBox->setMaximum(15);
};

inline void uses_delegate_config(QSpinBox* spinBox, const QModelIndex& index)
{
    Q_UNUSED(index);
    spinBox->setMinimum(0);
    spinBox->setMaximum(100); // Dunno?
};

CreatureAbilitiesSelector::CreatureAbilitiesSelector(nw::Creature* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::CreatureAbilitiesSelector)
    , obj_{obj}
{
    ui->setupUi(this);

    model_ = new CreatureAbilitiesModel(obj_, undoStack(), this);
    proxy_ = new CreatureAbilitiesSortFilterProxyModel(this);
    proxy_->setSourceModel(model_);
    ui->abilities->setModel(proxy_);
    ui->abilities->model()->sort(0);
    ui->abilities->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->abilities->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->abilities->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->abilities->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    level_delegate_ = new SpinBoxDelegate(level_delegate_config, this);
    ui->abilities->setItemDelegateForColumn(2, level_delegate_);
    uses_delegate_ = new SpinBoxDelegate(uses_delegate_config, this);
    ui->abilities->setItemDelegateForColumn(3, uses_delegate_);

    connect(ui->filter, &QLineEdit::textChanged, this, &::CreatureAbilitiesSelector::onFilterChanged);
    connect(ui->summary, &QPushButton::clicked, this, &CreatureAbilitiesSelector::onSummaryClicked);
}

CreatureAbilitiesSelector::~CreatureAbilitiesSelector()
{
    delete ui;
}

void CreatureAbilitiesSelector::onFilterChanged(const QString& text)
{
    proxy_->setFilterSpellName(text);
}

void CreatureAbilitiesSelector::onSummaryClicked()
{
    TextBoxDialog dialog("Special Abilities", generate_abilities_summary(obj_), 400, 500);
    dialog.exec();
}
