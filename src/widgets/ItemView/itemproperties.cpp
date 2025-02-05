#include "itemproperties.h"
#include "ui_itemproperties.h"

#include "../util/strings.h"
#include "itemview.h"
#include "proxymodels.h"
#include "statictwodamodel.h"

#include "nw/kernel/EffectSystem.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"

#include <QComboBox>
#include <QHelpEvent>
#include <QShortcut>
#include <QToolTip>

// == Undo Commands ===========================================================
// ============================================================================

class AddPropertyCommand : public QUndoCommand {
public:
    AddPropertyCommand(ItemPropertiesModel* model, const nw::ItemProperty& prop)
        : model_(model)
        , property_(prop)
    {
    }
    void undo() override
    {
        model_->removePropertyNoCmd(model_->rowCount() - 1);
    }
    void redo() override
    {
        model_->addPropertyNoCmd(property_);
    }

private:
    ItemPropertiesModel* model_;
    nw::ItemProperty property_;
};

class RemovePropertyCommand : public QUndoCommand {
public:
    RemovePropertyCommand(ItemPropertiesModel* model, int row)
        : model_(model)
        , row_(row)
        , property_(model_->getProperty(row))
    {
    }

    void undo() override
    {
        model_->insertPropertyNoCmd(row_, property_);
    }

    void redo() override
    {
        model_->removePropertyNoCmd(row_);
    }

private:
    ItemPropertiesModel* model_;
    int row_;
    nw::ItemProperty property_;
};

class ModifyPropertyCommand : public QUndoCommand {
public:
    ModifyPropertyCommand(ItemPropertiesModel* model, const QModelIndex& index,
        const QVariant& newValue, const QVariant& oldValue)
        : model_(model)
        , index_(index)
        , newValue_(newValue)
        , oldValue_(oldValue)
    {
    }

    void undo() override
    {
        model_->setDataNoCmd(index_, oldValue_);
    }

    void redo() override
    {
        model_->setDataNoCmd(index_, newValue_);
    }

private:
    ItemPropertiesModel* model_;
    QPersistentModelIndex index_;
    QVariant newValue_;
    QVariant oldValue_;
};

// == ItemPropertiesModel =====================================================
// ============================================================================

ItemPropertiesModel::ItemPropertiesModel(nw::Item* obj, QUndoStack* undo, QObject* parent)
    : QAbstractTableModel(parent)
    , obj_{obj}
    , undo_{undo}
{
}

void ItemPropertiesModel::addProperty(const nw::ItemProperty& prop)
{
    undo_->push(new AddPropertyCommand(this, prop));
}

void ItemPropertiesModel::addPropertyNoCmd(const nw::ItemProperty& prop)
{
    beginInsertRows(QModelIndex(), int(obj_->properties.size()), int(obj_->properties.size()));
    obj_->properties.push_back(prop);
    endInsertRows();
}

nw::ItemProperty ItemPropertiesModel::getProperty(int row) const
{
    if (size_t(row) > obj_->properties.size()) {
        return {};
    }
    return obj_->properties[row];
}

void ItemPropertiesModel::insertPropertyNoCmd(int row, const nw::ItemProperty& prop)
{
    beginInsertRows(QModelIndex(), row, row);
    obj_->properties.insert(std::begin(obj_->properties) + row, prop);
    endInsertRows();
}

void ItemPropertiesModel::removePropertyNoCmd(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    obj_->properties.erase(std::begin(obj_->properties) + row);
    endRemoveRows();
}

bool ItemPropertiesModel::setDataNoCmd(const QModelIndex& index, const QVariant& value)
{
    if (!index.isValid()) { return false; }
    auto& prop = obj_->properties[index.row()];

    switch (index.column()) {
    case 1:
        prop.subtype = uint16_t(value.toInt());
        break;
    case 2:
        prop.param_value = uint8_t(value.toInt());
        break;
    case 3:
        prop.cost_value = uint16_t(value.toInt());
        break;
    default:
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

int ItemPropertiesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) { return 0; }
    return int(obj_->properties.size());
}

int ItemPropertiesModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return 4;
}

QVariant ItemPropertiesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= int(obj_->properties.size())) {
        return {};
    }

    int temp_int;
    const auto& prop = obj_->properties[index.row()];
    if (role == Qt::UserRole) { return prop.type; }
    auto ipdef = nw::kernel::effects().ip_definition(nw::ItemPropertyType::make(prop.type));

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return to_qstring(nw::kernel::strings().get(ipdef->name));
        case 1: {
            auto row = size_t(prop.subtype);
            if (ipdef->subtype
                && row < ipdef->subtype->rows()
                && ipdef->subtype->get_to(row, "Name", temp_int)) {
                return to_qstring(nw::kernel::strings().get(temp_int));
            } else {
                return "-";
            }
        } break;
        case 2: {
            auto row = size_t(prop.param_value);
            if (ipdef->param_table
                && row < ipdef->param_table->rows()
                && ipdef->param_table->get_to(row, "Name", temp_int)) {
                return to_qstring(nw::kernel::strings().get(temp_int));
            } else {
                return "-";
            }

        } break;
        case 3: {
            auto row = size_t(prop.cost_value);
            if (ipdef->cost_table
                && row < ipdef->cost_table->rows()
                && ipdef->cost_table->get_to(row, "Name", temp_int)) {
                return to_qstring(nw::kernel::strings().get(temp_int));
            } else {
                return "-";
            }

        } break;
        default:
            return {};
        }
    } else if (role == Qt::EditRole) {
        switch (index.column()) {
        case 1:
            if (ipdef->subtype) { return prop.subtype; }
            break;
        case 2:
            if (ipdef->param_table) { return prop.param_value; }
            break;
        case 3:
            if (ipdef->cost_table) { return prop.cost_value; }
            break;
        default:
            return {};
        }
    }
    return {};
}

Qt::ItemFlags ItemPropertiesModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) { return Qt::NoItemFlags; }
    if (index.column() == 0) { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }

    const auto& prop = obj_->properties[index.row()];
    auto ipdef = nw::kernel::effects().ip_definition(nw::ItemPropertyType::make(prop.type));

    switch (index.column()) {
    case 1:
        if (!ipdef->subtype) { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
        break;
    case 2:
        if (!ipdef->param_table) { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
        break;
    case 3:
        if (!ipdef->cost_table) { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
        break;
    }

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ItemPropertiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Property");
        case 1:
            return tr("Subtype");
        case 2:
            return tr("Param");
        case 3:
            return tr("Cost");
        default:
            return {};
        }
    }
    return {};
}

bool ItemPropertiesModel::removeRows(int row, int count, const QModelIndex& parent = QModelIndex())
{
    Q_UNUSED(parent);

    if (row < 0 || row >= rowCount() || count <= 0 || row + count > rowCount()) {
        return false;
    }

    for (int i = 0; i < count; ++i) {
        undo_->push(new RemovePropertyCommand(this, row));
    }
    return true;
}

bool ItemPropertiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) { return false; }

    QVariant old;
    switch (index.column()) {
    case 1:
        old = int(obj_->properties[index.row()].subtype);
        if (old == value.toInt()) { return false; }
        break;
    case 2:
        old = int(obj_->properties[index.row()].param_value);
        if (old == value.toInt()) { return false; }
        break;
    case 3:
        old = int(obj_->properties[index.row()].cost_value);
        if (old == value.toInt()) { return false; }
        break;
    default:
        return false;
    }

    undo_->push(new ModifyPropertyCommand(this, index, value, old));
    return true;
}

// == ItemProperties ==========================================================
// ============================================================================

ItemProperties::ItemProperties(nw::Item* obj, ItemView* parent)
    : QWidget(parent)
    , ui(new Ui::ItemProperties)
    , obj_{obj}
    , undo_{new QUndoStack(this)}
{
    ui->setupUi(this);

    loadAllProperties();
    model_ = new ItemPropertiesModel(obj_, undo_, this);
    delegate_ = new ItemPropertyDelegate(this);
    ui->appliedProperties->setModel(model_);
    ui->appliedProperties->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->appliedProperties->setItemDelegate(delegate_);
    ui->appliedProperties->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::EditKeyPressed);
    auto filter = new ItemPropertiesEditFilter(ui->appliedProperties, this);
    ui->appliedProperties->viewport()->installEventFilter(filter);

    QAction* deleteAction = new QAction(this);
    deleteAction->setShortcut(QKeySequence::Delete);
    addAction(deleteAction);

    connect(deleteAction, &QAction::triggered, this, &ItemProperties::deleteSelectedRows);
    connect(ui->allProperties, &QListWidget::itemDoubleClicked, this, &ItemProperties::onItemPropertyDoubleClicked);

    QShortcut* us = new QShortcut(QKeySequence::Undo, this);
    QShortcut* rs = new QShortcut(QKeySequence::Redo, this);
    connect(us, &QShortcut::activated, undo_, &QUndoStack::undo);
    connect(rs, &QShortcut::activated, undo_, &QUndoStack::redo);
}

ItemProperties::~ItemProperties()
{
    delete ui;
}

void ItemProperties::loadAllProperties()
{
    auto itemprops = nw::kernel::effects().itemprops();
    if (!itemprops) { return; }
    auto bi_info = nw::kernel::rules().baseitems.get(obj_->baseitem);
    if (!bi_info || !bi_info->valid()) { return; }

    int i = 0, temp = 0;
    int col = bi_info->item_property_column;

    for (const auto& ipdef : nw::kernel::effects().ip_definitions()) {
        if (ipdef.name != 0xFFFFFFFF && itemprops->get_to(i, col, temp)) {
            auto item = new QListWidgetItem(to_qstring(nw::kernel::strings().get(ipdef.name)));
            item->setData(Qt::UserRole, i);
            ui->allProperties->addItem(item);
        }
        ++i;
    }
}

void ItemProperties::onBaseItemChanged(nw::BaseItem type)
{
    Q_UNUSED(type);
    ui->allProperties->clear();
    loadAllProperties();
}

void ItemProperties::onItemPropertyDoubleClicked(QListWidgetItem* item)
{
    nw::ItemProperty ip;
    ip.type = uint16_t(item->data(Qt::UserRole).toInt());
    model_->addProperty(ip);
}

void ItemProperties::deleteSelectedRows()
{
    QItemSelectionModel* selection = ui->appliedProperties->selectionModel();
    if (!selection) { return; }

    QModelIndexList selected = selection->selectedRows();
    std::sort(selected.begin(), selected.end(), [](const QModelIndex& a, const QModelIndex& b) {
        return a.row() > b.row();
    });

    foreach (const QModelIndex& index, selected) {
        model_->removeRows(index.row(), 1, QModelIndex());
    }
}

// == ItemPropertiesEditFilter ================================================
// ============================================================================

ItemPropertiesEditFilter::ItemPropertiesEditFilter(QTableView* view, QObject* parent)
    : QObject(parent)
    , m_view(view)
{
}

bool ItemPropertiesEditFilter::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        LOG_F(INFO, "Responding to event");
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = m_view->indexAt(mouseEvent->pos());
        if (index.isValid() && index.column() > 0) {
            m_view->edit(index);
            LOG_F(INFO, "Responded to event");
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

// == ItemPropertyDelegate ====================================================
// ============================================================================

ItemPropertyDelegate::ItemPropertyDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* ItemPropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto type = index.data(Qt::UserRole).toInt();
    auto ipdef = nw::kernel::effects().ip_definition(nw::ItemPropertyType::make(type));
    const nw::StaticTwoDA* twoda = nullptr;

    switch (index.column()) {
    case 1:
        if (ipdef->subtype) { twoda = ipdef->subtype; }
        break;
    case 2:
        if (ipdef->param_table) { twoda = ipdef->param_table; }
        break;
    case 3:
        if (ipdef->cost_table) { twoda = ipdef->cost_table; }
        break;
    }

    if (twoda) {
        auto name_column = int(twoda->column_index("Name"));
        auto combo = new QComboBox(parent);
        auto model = new StaticTwoDAModel(twoda, name_column, combo);
        model->setColumns({name_column});
        auto proxy = new EmptyFilterProxyModel(0, combo);
        proxy->setSourceModel(model);
        combo->setModel(proxy);
        return combo;
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

bool ItemPropertyDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::ToolTip) {
        QString text = index.data(Qt::DisplayRole).toString();
        QFontMetrics fm(view->font());
        QString elided = fm.elidedText(text, Qt::ElideRight, option.rect.width());

        if (elided != text) {
            QToolTip::showText(event->globalPos(), text, view);
        } else {
            QToolTip::hideText();
        }
        return true;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

void ItemPropertyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* combobox = static_cast<QComboBox*>(editor);
    if (!combobox) { return; }
    auto proxy = qobject_cast<const QSortFilterProxyModel*>(combobox->model());
    auto current = index.data(Qt::EditRole).toInt();
    int pindex = proxy ? proxy->mapFromSource(proxy->sourceModel()->index(current, 0)).row() : current;
    combobox->setCurrentIndex(pindex);
}

void ItemPropertyDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* combobox = static_cast<QComboBox*>(editor);
    if (!combobox) { return; }

    auto proxy = qobject_cast<const QSortFilterProxyModel*>(combobox->model());
    int selected = combobox->currentIndex();
    int value = proxy ? proxy->mapToSource(proxy->index(selected, 0)).row() : selected;
    model->setData(index, value, Qt::EditRole);
}
