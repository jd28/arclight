#include "encountercreaturemodel.h"

#include "../projectview.h"
#include "../util/strings.h"

#include "nw/kernel/Objects.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/objects/Encounter.hpp"
#

#include <QCoreApplication>
#include <QIODevice>
#include <QMimeData>

// == Undo Commands ===========================================================
// ============================================================================

class AddEncounterCreatureCommand : public QUndoCommand {
public:
    AddEncounterCreatureCommand(nw::Encounter* enc, const nw::SpawnCreature& creature,
        EncounterCreatureModel* model, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , enc_(enc)
        , creature_(creature)
        , model_{model}
    {
        setText(QString("Add creature %1").arg(QString::fromStdString(creature.name)));
    }

    void undo() override
    {
        model_->removeRows(int(enc_->creatures.size() - 1), 1);
    }

    void redo() override
    {
        model_->add(creature_);
    }

private:
    nw::Encounter* enc_;
    nw::SpawnCreature creature_;
    EncounterCreatureModel* model_;
};

RemoveEncounterCreatureCommand::RemoveEncounterCreatureCommand(nw::SpawnCreature creature, int index, EncounterCreatureModel* model, QUndoCommand* parent)
    : QUndoCommand(parent)
    , index_(index)
    , creature_(creature)
    , model_{model}
{
    setText(QString("Remove creature %1").arg(QString::fromStdString(creature_.name)));
}

void RemoveEncounterCreatureCommand::undo()
{
    model_->add(creature_, index_);
}

void RemoveEncounterCreatureCommand::redo()
{
    model_->removeRows(index_, 1);
}

class SetCreatureUniqueCommand : public QUndoCommand {
public:
    SetCreatureUniqueCommand(nw::Encounter* enc, int index, bool unique,
        EncounterCreatureModel* model, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , enc_(enc)
        , index_(index)
        , previous_(!unique)
        , next_(unique)
        , model_{model}
    {
        setText(QString("Set creature %1 unique flag to %2")
                .arg(QString::fromStdString(enc->creatures[index].name))
                .arg(unique));
    }

    void undo() override
    {
        enc_->creatures[index_].single_spawn = previous_;
        QModelIndex idx = model_->index(index_, 4);
        emit model_->dataChanged(idx, idx, {Qt::EditRole});
    }

    void redo() override
    {
        enc_->creatures[index_].single_spawn = next_;
        QModelIndex idx = model_->index(index_, 4);
        emit model_->dataChanged(idx, idx, {Qt::EditRole});
    }

private:
    nw::Encounter* enc_;
    int index_;
    bool previous_;
    bool next_;
    EncounterCreatureModel* model_;
};

// == EncounterCreatureModel ==================================================
// ============================================================================

EncounterCreatureModel::EncounterCreatureModel(nw::Encounter* obj, QUndoStack* undo, QObject* parent)
    : QAbstractTableModel(parent)
    , obj_{obj}
    , undo_{undo}
{
    nw::String temp;
    for (auto& it : obj_->creatures) {
        auto cre = nw::kernel::objects().load<nw::Creature>(it.resref);
        if (!cre) { continue; }
        it.name = nw::kernel::strings().get(cre->name_first);
        temp = nw::kernel::strings().get(cre->name_last);
        if (!temp.empty()) { it.name += " " + temp; }
        it.tag = cre->tag();
        nw::kernel::objects().destroy(cre->handle());
    }
}

void EncounterCreatureModel::add(nw::SpawnCreature creature, int index)
{
    if (index == -1) {
        beginInsertRows(QModelIndex(), int(obj_->creatures.size()), int(obj_->creatures.size()));
        obj_->creatures.push_back(std::move(creature));
        endInsertRows();
    } else {
        beginInsertRows(QModelIndex(), index, index);
        obj_->creatures.insert(obj_->creatures.begin() + index, std::move(creature));
        endInsertRows();
    }

    emit layoutChanged();
}

bool EncounterCreatureModel::canDropMimeData(const QMimeData* data,
    Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(column);
    Q_UNUSED(row);
    Q_UNUSED(parent);

    if (!data->hasFormat("application/x-arclight-projectitem")) { return false; }
    QByteArray data_ = data->data("application/x-arclight-projectitem");
    QDataStream stream(&data_, QIODevice::ReadOnly);

    qint64 senderPid;
    stream >> senderPid;
    if (senderPid != QCoreApplication::applicationPid()) { return false; }

    qlonglong ptr;
    stream >> ptr;
    const ProjectItem* node = reinterpret_cast<const ProjectItem*>(ptr);
    if (!node) { return false; }

    if (node->res_.type != nw::ResourceType::utc) { return false; }

    return true;
}

int EncounterCreatureModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant EncounterCreatureModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return obj_->creatures[index.row()].cr;
        case 1:
            return to_qstring(obj_->creatures[index.row()].name);
        case 2:
            return to_qstring(obj_->creatures[index.row()].tag.view());
        case 3:
            return to_qstring(obj_->creatures[index.row()].resref.view());
        case 4:
            return obj_->creatures[index.row()].single_spawn;
        }
    }
    return {};
}

bool EncounterCreatureModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
    int row, int column, const QModelIndex& parent)
{
    Q_UNUSED(action);
    Q_UNUSED(column);
    Q_UNUSED(row);
    Q_UNUSED(parent);

    if (!data->hasFormat("application/x-arclight-projectitem")) { return false; }

    QByteArray data_ = data->data("application/x-arclight-projectitem");
    QDataStream stream(&data_, QIODevice::ReadOnly);

    qint64 senderPid;
    stream >> senderPid;
    if (senderPid != QCoreApplication::applicationPid()) { return false; }

    qlonglong ptr;
    stream >> ptr;
    const ProjectItem* node = reinterpret_cast<const ProjectItem*>(ptr);
    if (!node) { return false; }

    if (node->res_.type != nw::ResourceType::utc) { return false; }

    auto cre = nw::kernel::objects().load<nw::Creature>(node->res_.resref);
    if (!cre) { return false; }

    nw::SpawnCreature creature;
    creature.cr = cre->cr;
    creature.resref = node->res_.resref;
    creature.name = nw::kernel::strings().get(cre->name_first);
    nw::String temp = nw::kernel::strings().get(cre->name_last);
    if (!temp.empty()) { creature.name += " " + temp; }
    creature.tag = cre->tag();
    nw::kernel::objects().destroy(cre->handle());
    creature.single_spawn = false;

    undo_->push(new AddEncounterCreatureCommand(obj_, creature, this));

    return true;
}

Qt::ItemFlags EncounterCreatureModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags result = QAbstractItemModel::flags(index);
    result |= Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    if (index.column() == 4) {
        result |= Qt::ItemIsEditable;
    }
    return result;
}

QVariant EncounterCreatureModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        default:
            return {};
        case 0:
            return "CR";
        case 1:
            return "Name";
        case 2:
            return "Tag";
        case 3:
            return "Resref";
        case 4:
            return "Unique";
        }
    }

    return {};
}

QModelIndex EncounterCreatureModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) { return {}; }
    return createIndex(row, column);
}

QModelIndex EncounterCreatureModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return {};
}

bool EncounterCreatureModel::removeRows(int row, int count, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    obj_->creatures.erase(
        obj_->creatures.begin() + row,
        obj_->creatures.begin() + row + count);
    endRemoveRows();
    return true;
}

int EncounterCreatureModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) { return 0; }
    return int(obj_->creatures.size());
}

bool EncounterCreatureModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) { return false; }
    if (index.column() != 4) { return false; }
    undo_->push(new SetCreatureUniqueCommand(obj_, index.row(), value.toBool(), this));
    return true;
}

Qt::DropActions EncounterCreatureModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
