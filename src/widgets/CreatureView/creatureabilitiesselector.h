#ifndef CREATUREABILITIESSELECTOR_H
#define CREATUREABILITIESSELECTOR_H

#include "nw/rules/Class.hpp"

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QWidget>

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureAbilitiesSelector;
}

class SpinBoxDelegate;

// == CreatureAbilitiesModel ==================================================
// ============================================================================

class CreatureAbilitiesModel : public QAbstractTableModel {
    Q_OBJECT

public:
    CreatureAbilitiesModel(nw::Creature* creature, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

private:
    nw::Creature* creature_;
};

// == CreatureAbilitiesSortFilterProxyModel ===================================
// ============================================================================

class CreatureAbilitiesSortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit CreatureAbilitiesSortFilterProxyModel(QObject* parent = nullptr);

    void setFilterSpellName(const QString& name);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    QString name_;
};

// == CreatureAbilitiesSelector ============================================
// ============================================================================

class CreatureAbilitiesSelector : public QWidget {
    Q_OBJECT

public:
    explicit CreatureAbilitiesSelector(nw::Creature* obj, QWidget* parent = nullptr);
    ~CreatureAbilitiesSelector();

private slots:
    void onFilterChanged(const QString& text);
    void onSummaryClicked();

private:
    Ui::CreatureAbilitiesSelector* ui;
    nw::Creature* obj_;
    CreatureAbilitiesModel* model_ = nullptr;
    CreatureAbilitiesSortFilterProxyModel* proxy_ = nullptr;
    SpinBoxDelegate* level_delegate_ = nullptr;
    SpinBoxDelegate* uses_delegate_ = nullptr;
};

#endif // CREATUREABILITIESSELECTOR_H
