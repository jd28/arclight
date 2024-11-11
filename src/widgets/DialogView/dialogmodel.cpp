#include "dialogmodel.h"

#include "../util/strings.h"

#include "nw/formats/Dialog.hpp"
#include "nw/kernel/Strings.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QFont>
#include <QIODevice>
#include <QMimeData>

// == DialogItem ==============================================================
// ============================================================================

DialogItem::DialogItem(nw::DialogPtr* ptr, int row, DialogModel* model, AbstractTreeItem* parent)
    : AbstractTreeItem(row, parent)
    , ptr_(ptr)
    , model_{model}
{
    if (!ptr || ptr_->is_link) { return; }

    for (size_t i = 0; i < ptr_->node->pointers.size(); ++i) {
        appendChild(new DialogItem(ptr_->node->pointers[i], int(i), model_, this));
    }
}

QVariant DialogItem::data(int column, int role) const
{
    Q_UNUSED(role);
    if (!ptr_) { return "Root"; }
    nw::DialogNode* n = ptr_->node;
    switch (column) {
    default:
        return QVariant();
    case 0:
        return to_qstring(nw::kernel::strings().get(n->text, model_->feminine_)).trimmed();
    }
}

// == DialogModel =============================================================
// ============================================================================

DialogModel::DialogModel(nw::Dialog* dialog, QObject* parent)
    : AbstractTreeModel(parent)
    , dialog_{dialog}
    , font_{QApplication::font()}
{
}

DialogModel::~DialogModel()
{
}

bool DialogModel::canDropMimeData(const QMimeData* mimeData, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    // Note parent parameter if dropping direcly on to a node, is that node, i.e. the new parent.
    // If dropping between nodes it's the parent of the new siblings.

    if (!mimeData->hasFormat("application/x-dialogitem")) {
        return false;
    }

    QByteArray data = mimeData->data("application/x-dialogitem");
    QDataStream stream(&data, QIODevice::ReadOnly);
    qint64 senderPid;
    stream >> senderPid;
    if (senderPid != QCoreApplication::applicationPid()) {
        // Let's not cast pointers that come from another process...
        return false;
    }

    qlonglong ptr;
    stream >> ptr;
    const DialogItem* node = reinterpret_cast<const DialogItem*>(ptr);

    const DialogItem* parent_node = index_to_node(parent);
    Q_ASSERT(parent_node);

    bool parent_is_root = parent_node == root();

    LOG_F(INFO, "is root: {}, row: {}, col: {}, parent: {}", parent_is_root, row, column,
        !parent_is_root ? parent_node->ptr_->node->text.get(nw::LanguageID::english) : "Root");

    // Don't support in between drops, it's too confusing UX the way Qt supports this.
    if (row != -1) { return false; }

    if (parent_is_root) {
        // Only accept drops on the root node itself that are entries.
        // NOTE: The toolset allows moving links to root but not pasting them..
        // Not sure if it's best to disallow that.
        if (node->ptr_->type == nw::DialogNodeType::reply) {
            return false;
        }
    } else {
        // No between node drops if the parent and node have the same node type
        // UNLESS they have the same parent in which case we reorder.
        if (parent_node->ptr_->type == node->ptr_->type) {
            return parent_node->parent_ == node->parent_;
        } else if (parent_node == node->parent_) { // We're already there..
            return false;
        } else if (parent_node->ptr_->is_link) { // No dropping on links..
            return false;
        }
    }

    // Everything else will be ok and handled by moving / reordering.

    return AbstractTreeModel::canDropMimeData(mimeData, action, row, column, parent);
}

int DialogModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant DialogModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    DialogItem* node = static_cast<DialogItem*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        return node->data(index.column());
    } else if (role == Qt::FontRole) {
        return font_;
    } else if (role == Qt::ForegroundRole) {
        if (!node->ptr_) { return QVariant(link_); }
        nw::DialogPtr* p = node->ptr_;
        if (p->is_link) {
            return QVariant(link_);
        } else if (p->type == nw::DialogNodeType::entry) {
            return QVariant(entry_);
        } else if (p->type == nw::DialogNodeType::reply) {
            return QVariant(reply_);
        }
    }

    return QVariant();
}

nw::Dialog* DialogModel::dialog()
{
    return dialog_.get();
}

bool DialogModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_ASSERT(action == Qt::MoveAction);
    Q_UNUSED(column);
    Q_UNUSED(row);

    if (!mimeData->hasFormat("application/x-dialogitem")) {
        return false;
    }

    QByteArray data = mimeData->data("application/x-dialogitem");
    QDataStream stream(&data, QIODevice::ReadOnly);
    qint64 senderPid;
    stream >> senderPid;
    if (senderPid != QCoreApplication::applicationPid()) {
        // Let's not cast pointers that come from another process...
        return false;
    }

    DialogItem* parent_node = index_to_node(parent);
    Q_ASSERT(parent_node);
    bool parent_is_root = parent_node == root();

    qlonglong ptr;
    stream >> ptr;
    DialogItem* node = reinterpret_cast<DialogItem*>(ptr);

    if (parent_is_root) {
        beginRemoveRows(createIndex(node->row_, 0, node->parent_), node->row_, node->row_);
        node->parent_->removeChild(node);
        node->ptr_->parent->remove_ptr(node->ptr_);
        endRemoveRows();

        beginInsertRows(parent, parent_node->childCount(), parent_node->childCount() + 1);
        node->parent_ = parent_node;
        parent_node->appendChild(node);
        dialog_->add_ptr(node->ptr_); // parent_node is a fake root node, have to add to dialog starts
        endInsertRows();
        return true;
    } else if (parent_node->ptr_->type != node->ptr_->type) { // Move from one parent to new
        beginRemoveRows(createIndex(node->row_, 0, node->parent_), node->row_, node->row_);
        node->parent_->removeChild(node);
        node->ptr_->parent->remove_ptr(node->ptr_);
        endRemoveRows();

        beginInsertRows(parent, parent_node->childCount(), parent_node->childCount() + 1);
        node->parent_ = parent_node;
        parent_node->appendChild(node);
        parent_node->ptr_->add_ptr(node->ptr_);
        endInsertRows();
        return true;
    } else if (parent_node->parent_ == node->parent_) { // We're dropping on a node of the same type with the same parent
        DialogItem* pr = reinterpret_cast<DialogItem*>(parent_node->parent_);
        size_t insert_at = node->row_ < parent_node->row_ ? parent_node->row_ - 1 : parent_node->row_ + 1;
        pr->ptr_->node->pointers.erase(std::begin(pr->ptr_->node->pointers) + node->row_);
        pr->ptr_->node->pointers.insert(std::begin(pr->ptr_->node->pointers) + insert_at, node->ptr_);

        beginRemoveRows(parent.parent(), node->row_, node->row_);
        parent_node->parent_->children_.removeAll(node);
        endRemoveRows();

        beginInsertRows(parent.parent(), parent_node->row_, parent_node->row_);
        parent_node->parent_->children_.insert(parent_node->row_, node);
        endInsertRows();

        parent_node->parent_->refreshChildRows();
    }

    return false;
}

Qt::ItemFlags DialogModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant DialogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

DialogItem* DialogModel::index_to_node(QModelIndex index)
{
    if (!index.isValid()) { return static_cast<DialogItem*>(root()); }
    return static_cast<DialogItem*>(index.internalPointer());
}

DialogItem* DialogModel::index_to_node(QModelIndex index) const
{
    if (!index.isValid()) { return static_cast<DialogItem*>(root()); }
    return static_cast<DialogItem*>(index.internalPointer());
}

void DialogModel::loadRootItems()
{
    if (!dialog_) { return; }
    auto root_item = new DialogItem(nullptr, 0, this);
    addRootItem(root_item);
    int row = 0;
    for (auto& start : dialog_->starts) {
        root_item->appendChild(new DialogItem(start, row, this, root_item));
        ++row;
    }
}

QMimeData* DialogModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    const DialogItem* node = index_to_node(indexes[0]);

    stream << QCoreApplication::applicationPid();
    stream << reinterpret_cast<qlonglong>(node);

    mimeData->setData("application/x-dialogitem", data);
    return mimeData;
}

QStringList DialogModel::mimeTypes() const
{
    return QStringList() << "application/x-dialogitem";
}

void DialogModel::setColors(QColor entry, QColor reply, QColor link)
{
    entry_ = entry;
    reply_ = reply;
    link_ = link;
}

void DialogModel::setFont(const QFont& font)
{
    font_ = font;
    QModelIndex topLeft = createIndex(0, 0);
    emit dataChanged(topLeft, topLeft);
}

void DialogModel::setLanguage(nw::LanguageID lang, bool feminine)
{
    lang_ = lang;
    feminine_ = feminine;
    QModelIndex topLeft = createIndex(0, 0);
    emit dataChanged(topLeft, topLeft);
}

Qt::DropActions DialogModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions DialogModel::supportedDragActions() const
{
    return Qt::MoveAction;
}
