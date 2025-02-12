#pragma once

#include "../arclighttab.h"

#include "nw/objects/SpellBook.hpp"
#include "nw/rules/Class.hpp"

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QUndoCommand>

// == Forward Decla ===========================================================
// ============================================================================

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureSpellSelector;
}

class CheckBoxDelegate;
class SpinBoxDelegate;
class CreatureSpellSelector;
class CreatureSpellModel;

// == Undo Commands ===========================================================
// ============================================================================

class ModifySpellCommand : public QUndoCommand {
public:
    ModifySpellCommand(CreatureSpellSelector* selector, CreatureSpellModel* model, nw::Creature* creature,
        nw::Class cls, nw::Spell spell, nw::MetaMagicFlag metamagic, int previous, int next,
        int spellLevel, int currentFilterLevel, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void updateSpell(int previous, int next);

    CreatureSpellSelector* selector_;
    CreatureSpellModel* model_;
    nw::Creature* creature_;
    nw::Class class_;
    nw::Spell spell_;
    nw::MetaMagicFlag metamagic_;
    int previous_;
    int next_;
    int spellLevel_;
    int currentFilterLevel_;
    int row_;
};

class ClearSpellsCommand : public QUndoCommand {
public:
    ClearSpellsCommand(CreatureSpellSelector* selector, CreatureSpellModel* model, nw::Creature* creature,
        nw::Class cls, int spellLevel, nw::SpellBook previous, nw::SpellBook next, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    struct SpellState {
        nw::Spell spell;
        nw::MetaMagicFlag metamagic;
        int count;
    };

    void notifyModelRangeChanged();

    CreatureSpellSelector* selector_;
    CreatureSpellModel* model_;
    nw::Creature* creature_;
    nw::Class class_;
    int spell_level_;
    nw::SpellBook previous_;
    nw::SpellBook next_;
};

// == CreatureSpellModel ======================================================
// ============================================================================

class CreatureSpellModel : public QAbstractTableModel {
    Q_OBJECT

public:
    CreatureSpellModel(nw::Creature* creature, nw::Class cls, CreatureSpellSelector* parent);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

    int getRemainingSlots(int currentRow = -1) const;
    void setClass(nw::Class cls);
    void setMetaMagic(nw::MetaMagicFlag meta);
    void setSpellLevel(int level);

private:
    nw::Creature* creature_ = nullptr;
    nw::Class class_ = nw::Class::invalid();
    nw::MetaMagicFlag metamagic_ = nw::metamagic_none;
    CreatureSpellSelector* parent_ = nullptr;
    int filter_level_ = 0;
};

// == CreatureSpellSortFilterProxyModel =======================================
// ============================================================================

class CreatureSpellSortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit CreatureSpellSortFilterProxyModel(QObject* parent = nullptr);

    void setFilterSpellName(const QString& name);
    void setFilterSpellLevel(int level);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QString name_;
    int level_ = -1;
};

class CreatureSpellSelector : public ArclightTab {
    Q_OBJECT

public:
    explicit CreatureSpellSelector(nw::Creature* creature, ArclightView* parent = nullptr);
    ~CreatureSpellSelector();

    void refreshModel();
    void setClass(nw::Class cls);
    void setMetamagic(nw::MetaMagicFlag metamagic);
    void setSpell(nw::Spell spell);
    void setSpellFilterLevel(int level);

    bool eventFilter(QObject* object, QEvent* event);

private slots:
    void onClassChanged(int currentIndex);
    void onClearClicked();
    void onFilterChanged(const QString& text);
    void onMetaMagicChanged(int currentIndex);
    void onSpellLevelChanged(int currentIndex);
    void onSummaryClicked();

private:
    Ui::CreatureSpellSelector* ui;
    nw::Creature* creature_;
    nw::Class class_ = nw::Class::invalid();
    nw::MetaMagicFlag metamagic_ = nw::metamagic_none;
    CreatureSpellModel* model_ = nullptr;
    CreatureSpellSortFilterProxyModel* proxy_ = nullptr;
    CheckBoxDelegate* known_delegate_ = nullptr;
    SpinBoxDelegate* memorize_delegate_ = nullptr;
};
