#pragma once

#include "../arclighttab.h"

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureFeatSelector;
}

class ArclightView;

// == CreatureFeatSelectorSortFilterProxy =====================================
// ============================================================================

class CreatureFeatSelectorSortFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit CreatureFeatSelectorSortFilterProxy(QObject* parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

public slots:
    void onFilterUpdated(const QString& filter);

private:
    std::string filter_;
    bool filter_empty_ = true;
};

// == CreatureFeatSelectorModel ==============================================
// ============================================================================

class CreatureFeatSelectorModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit CreatureFeatSelectorModel(nw::Creature* creature, QObject* parent = nullptr);

    void setUndoStack(QUndoStack* undo);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

signals:
    void featsChanged();

private:
    nw::Creature* creature_ = nullptr;
    QUndoStack* undo_;
};

// == CreatureFeatSelector ====================================================
// ============================================================================

class CreatureFeatSelector : public ArclightTab {
    Q_OBJECT

public:
    explicit CreatureFeatSelector(nw::Creature* creature, ArclightView* parent = nullptr);
    ~CreatureFeatSelector();

    bool eventFilter(QObject* object, QEvent* event);
    CreatureFeatSelectorModel* model() const;

private:
    Ui::CreatureFeatSelector* ui;
    CreatureFeatSelectorModel* model_ = nullptr;
    CreatureFeatSelectorSortFilterProxy* filter_ = nullptr;
    nw::Creature* creature_ = nullptr;
};
