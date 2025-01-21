#ifndef CREATURESPELLSELECTOR_H
#define CREATURESPELLSELECTOR_H

#include "nw/rules/Class.hpp"

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QWidget>

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureSpellSelector;
}

class CheckBoxDelegate;
class SpinBoxDelegate;

// == CreatureSpellModel ======================================================
// ============================================================================

class CreatureSpellModel : public QAbstractTableModel {
    Q_OBJECT

public:
    CreatureSpellModel(nw::Creature* creature, nw::Class cls, QObject* parent = nullptr);

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

private:
    nw::Creature* creature_;
    nw::Class class_ = nw::Class::invalid();
    nw::MetaMagicFlag metamagic_ = nw::metamagic_none;
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

class CreatureSpellSelector : public QWidget {
    Q_OBJECT

public:
    explicit CreatureSpellSelector(nw::Creature* creature, QWidget* parent = nullptr);
    ~CreatureSpellSelector();

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

#endif // CREATURESPELLSELECTOR_H
