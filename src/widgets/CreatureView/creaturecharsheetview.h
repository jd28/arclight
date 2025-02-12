#pragma once

#include "../arclighttab.h"

#include <QSortFilterProxyModel>

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Class;
struct Creature;
}

namespace Ui {
class CreatureCharSheetView;
}

class CreatureView;

// == CreatureClassFilter =====================================================
// ============================================================================

class CreatureClassFilter : public QSortFilterProxyModel {
public:
    CreatureClassFilter(nw::Creature* obj, int slot, QObject* parent = nullptr);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    nw::Creature* obj_;
    int slot_;
};

// == CreaturePackageFilter ===================================================
// ============================================================================

class CreaturePackageFilter : public QSortFilterProxyModel {
public:
    CreaturePackageFilter(nw::Creature* obj, QObject* parent = nullptr);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    nw::Creature* obj_;
};

// == CreatureCharSheetView ===================================================
// ============================================================================

class CreatureCharSheetView : public ArclightTab {
    Q_OBJECT

public:
    explicit CreatureCharSheetView(nw::Creature* obj, CreatureView* parent = nullptr);
    ~CreatureCharSheetView();

    void loadCreature(nw::Creature* obj);
    void loadPortrait(nw::Creature* obj);

public slots:
    void onReloadStats();

signals:
    void classAdded(nw::Class class_);
    void classRemoved(nw::Class class_);

private slots:
    void onClassChanged(int index);
    void onClassDeleteButtonClicked();
    void onClassLevelChanged(int value);

private:
    void loadStatsAbilities();
    void loadStatsSaves();

    Ui::CreatureCharSheetView* ui;
    nw::Creature* obj_ = nullptr;
    CreaturePackageFilter* pkg_filter_;
    QList<CreatureClassFilter*> cls_filters_;
};
