#pragma once

#include "nw/objects/Encounter.hpp"

#include <QAbstractTableModel>
#include <QUndoStack>

class EncounterCreatureModel;

class RemoveEncounterCreatureCommand : public QUndoCommand {
public:
    RemoveEncounterCreatureCommand(nw::SpawnCreature creature, int index,
        EncounterCreatureModel* model, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int index_;
    nw::SpawnCreature creature_;
    EncounterCreatureModel* model_;
};

// == EncounterCreatureModel ==================================================
// ============================================================================

class EncounterCreatureModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit EncounterCreatureModel(nw::Encounter* obj, QUndoStack* undo, QObject* parent = nullptr);

    void add(nw::SpawnCreature creature, int index = -1);

    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action,
        int row, int column, const QModelIndex& parent) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::DropActions supportedDropActions() const override;

private:
    nw::Encounter* obj_;
    QUndoStack* undo_;
};
