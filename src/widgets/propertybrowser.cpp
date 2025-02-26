#include "propertybrowser.h"

#include "nw/log.hpp"
#include "xxhash/xxh3.h"

#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QTimer>
#include <QUndoStack>

// == Undo Commands ===========================================================
// ============================================================================

class PropertyValueCommand : public QUndoCommand {
public:
    PropertyValueCommand(Property* property, const QVariant& oldValue,
        const QVariant& newValue, PropertyModel* model,
        QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , property_(property)
        , oldValue_(oldValue)
        , newValue_(newValue)
        , model_(model)
    {
        setText(QString("Change %1").arg(property->name));
    }

    int id() const override
    {
        uint64_t hash = XXH3_64bits(reinterpret_cast<const void*>(property_), sizeof(property_));
        return static_cast<int>((hash >> 32) ^ (hash & 0xFFFFFFFF));
    }

    bool mergeWith(const QUndoCommand* other) override
    {
        const PropertyValueCommand* cmd = dynamic_cast<const PropertyValueCommand*>(other);
        if (!cmd) { return false; }
        if (property_ != cmd->property_) { return false; }
        newValue_ = cmd->newValue_;
        return true;
    }

    void undo() override
    {
        QModelIndex index = model_->indexForProperty(property_);
        if (!index.isValid()) { return; }
        property_->value = oldValue_;
        if (property_->on_set) { property_->on_set(oldValue_); }
        emit model_->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }

    void redo() override
    {
        QModelIndex index = model_->indexForProperty(property_);
        if (!index.isValid()) { return; }
        property_->value = newValue_;
        if (property_->on_set) { property_->on_set(newValue_); }
        emit model_->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }

private:
    Property* property_;
    QVariant oldValue_;
    QVariant newValue_;
    PropertyModel* model_;
};

// == Property ================================================================
// ============================================================================

Property::~Property()
{
    removeFromParent();
    foreach (auto child, children) {
        delete child;
    }
}

void Property::removeFromParent()
{
    if (parent) {
        parent->children.removeAll(this);
        parent = nullptr;
    }
}

void Property::setEditable(bool val)
{
    setReadOnly(!val);
}

void Property::setReadOnly(bool val)
{
    read_only = val;
}

// == PropertyDelegate ========================================================
// ============================================================================

PropertyDelegate::PropertyDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* PropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Property* prop = qvariant_cast<Property*>(index.data(PropertyModel::PropertyRole));
    if (!prop) { return nullptr; }

    switch (prop->type) {
    case PropertyType2::Boolean: {
        auto ed = new AutoCommitCheckBox(parent);
        return ed;
    }
    case PropertyType2::Enum: {
        if (prop->dialog) {
            auto editor = prop->dialog(prop, parent);
            if (editor) {
                if (QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model())) {
                    model->setData(index, editor->property("value"));
                }
                editor->deleteLater();
            }
            return nullptr;
        }

        AutoCommitComboBox* combo = new AutoCommitComboBox(parent);
        if (prop->enum_config.model) {
            combo->setModel(prop->enum_config.model);
        }
        combo->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        combo->view()->setTextElideMode(Qt::ElideRight);
        return combo;
    }
    case PropertyType2::Integer: {
        auto ed = new AutoCommitSpinBox(parent);
        if (prop->int_config.min.isValid()) {
            ed->setMinimum(prop->int_config.min.toInt());
        }
        if (prop->int_config.max.isValid()) {
            ed->setMaximum(prop->int_config.max.toInt());
        }
        if (prop->int_config.step.isValid()) {
            ed->setSingleStep(prop->int_config.step.toInt());
        }
        return ed;
    }
    case PropertyType2::Double: {
        auto ed = new AutoCommitDoubleSpinBox(parent);
        if (prop->double_config.min.isValid()) {
            ed->setMinimum(prop->double_config.min.toDouble());
        }
        if (prop->double_config.max.isValid()) {
            ed->setMaximum(prop->double_config.max.toDouble());
        }
        if (prop->double_config.step.isValid()) {
            ed->setSingleStep(prop->double_config.step.toDouble());
        }
        return ed;
    }
    case PropertyType2::String: {
        auto ed = new AutoCommitLineEdit(parent);
        if (prop->string_config.completer) {
            ed->setCompleter(prop->string_config.completer);
        }
        if (prop->string_config.validator) {
            ed->setValidator(prop->string_config.validator);
        }

        return ed;
    }

    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void PropertyDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Property* prop = qvariant_cast<Property*>(index.data(PropertyModel::PropertyRole));

    if (prop && prop->type == PropertyType2::Boolean) {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        bool isChecked = prop->value.toBool();
        QStyleOptionButton checkboxOption;
        QRect checkboxRect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxOption, opt.widget);
        checkboxOption.rect = QRect(opt.rect.left(), opt.rect.center().y() - checkboxRect.height() / 2,
            checkboxRect.width(), checkboxRect.height());
        checkboxOption.state = QStyle::State_Enabled | (isChecked ? QStyle::State_On : QStyle::State_Off);
        style->drawControl(QStyle::CE_CheckBox, &checkboxOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

void PropertyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    Property* prop = qvariant_cast<Property*>(index.data(PropertyModel::PropertyRole));

    switch (prop->type) {
    default:
        QStyledItemDelegate::setEditorData(editor, index);
        break;
    case PropertyType2::Boolean: {
        auto ed = qobject_cast<AutoCommitCheckBox*>(editor);
        ed->setChecked(prop->value.toBool());
        ed->setInitialized();
        break;
    }
    case PropertyType2::Enum: {
        if (AutoCommitComboBox* combo = qobject_cast<AutoCommitComboBox*>(editor)) {
            combo->setCurrentIndex(prop->value.toInt());
            combo->setInitialized();
        }
        break;
    }
    case PropertyType2::Integer: {
        auto ed = qobject_cast<AutoCommitSpinBox*>(editor);
        ed->setValue(prop->value.toInt());
        ed->setInitialized();
        break;
    }
    case PropertyType2::String: {
        if (auto ed = qobject_cast<AutoCommitLineEdit*>(editor)) {
            ed->setText(prop->value.toString());
            ed->setInitialized();
        }
        break;
    }
    case PropertyType2::Double: {
        auto ed = qobject_cast<AutoCommitDoubleSpinBox*>(editor);
        ed->setValue(prop->value.toDouble());
        ed->setInitialized();
        break;
    }
    }
}

void PropertyDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    Property* prop = qvariant_cast<Property*>(index.data(PropertyModel::PropertyRole));
    switch (prop->type) {
    default:
        QStyledItemDelegate::setModelData(editor, model, index);
        break;
    case PropertyType2::Boolean: {
        auto ed = qobject_cast<AutoCommitCheckBox*>(editor);
        model->setData(index, ed->isChecked());
        break;
    }
    case PropertyType2::Enum: {
        if (auto combo = qobject_cast<AutoCommitComboBox*>(editor)) {
            model->setData(index, combo->currentIndex());
        } else if (auto ed = qobject_cast<QLabel*>(editor)) {
            model->setData(index, ed->property("value"));
        }
        break;
    }
    case PropertyType2::Integer: {
        auto ed = qobject_cast<AutoCommitSpinBox*>(editor);
        model->setData(index, ed->value());
        break;
    }
    case PropertyType2::String: {
        auto ed = qobject_cast<AutoCommitLineEdit*>(editor);
        model->setData(index, ed->text());
        break;
    }
    case PropertyType2::Double: {
        auto ed = qobject_cast<AutoCommitDoubleSpinBox*>(editor);
        model->setData(index, ed->value());
        break;
    }
    }
}

QSize PropertyDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
#ifdef Q_OS_WINDOWS
    size.setHeight(size.height() + 6);
#endif
    return size;
}

// == PropertyModel ===========================================================
// ============================================================================

PropertyModel::PropertyModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

PropertyModel::~PropertyModel()
{
    foreach (auto prop, properties_) {
        delete prop;
    }
}

void PropertyModel::addProperty(Property* prop, Property* parent)
{
    QList<Property*>* properties = parent
        ? &parent->children
        : &properties_;

    int row = int(properties->size());
    QModelIndex pindex = parent
        ? createIndex(int(properties_.indexOf(parent)), 0, parent)
        : QModelIndex();

    beginInsertRows(pindex, row, row);
    prop->parent = parent;
    properties->append(prop);
    endInsertRows();
}

void PropertyModel::deleteProperty(Property* prop)
{
    if (!prop) { return; }

    QModelIndex index = indexForProperty(prop);
    if (index.isValid()) {
        if (prop->parent) {
            int row = int(prop->parent->children.indexOf(prop));
            if (row >= 0) {
                beginRemoveRows(indexForProperty(prop->parent), row, row);
                prop->parent->children.removeAt(row);
                endRemoveRows();
            }
        } else {
            int row = int(properties_.indexOf(prop));
            if (row >= 0) {
                beginRemoveRows(QModelIndex(), row, row);
                properties_.removeAt(row);
                endRemoveRows();
            }
        }
    }
    delete prop;
}

QModelIndex PropertyModel::indexForProperty(Property* prop) const
{
    if (!prop) { return QModelIndex(); }

    int row;
    if (prop->parent) {
        row = int(prop->parent->children.indexOf(prop));
        if (row == -1) { return QModelIndex(); }
    } else {
        row = int(properties_.indexOf(prop));
        if (row == -1) { return QModelIndex(); }
    }

    return createIndex(row, ColumnValue, prop);
}

void PropertyModel::removeProperty(Property* prop)
{
    if (!prop) { return; }

    Property* parent = prop->parent;
    QList<Property*>* properties = parent ? &parent->children : &properties_;

    int row = int(properties->indexOf(prop));
    if (row == -1) { return; }

    QModelIndex parentIndex = parent ? indexForProperty(parent) : QModelIndex();

    beginRemoveRows(parentIndex, row, row);
    properties->removeAt(row);
    endRemoveRows();

    prop->removeFromParent();
    delete prop;
}

void PropertyModel::replaceProperty(Property* old, Property* replacement)
{
    if (!old || !replacement) { return; }

    Property* parent = old->parent;

    QList<Property*>* properties = parent ? &parent->children : &properties_;

    int row = int(properties->indexOf(old));
    if (row == -1) { return; }

    QModelIndex parentIndex = parent ? indexForProperty(parent) : QModelIndex();

    beginInsertRows(parentIndex, row, row);
    properties->insert(row, replacement);
    replacement->parent = parent;
    endInsertRows();

    removeProperty(old);
}

void PropertyModel::setUndoStack(QUndoStack* undo)
{
    undo_ = undo;
}

void PropertyModel::updateReadOnly(Property* prop)
{
    QModelIndex index = indexForProperty(prop);
    if (!index.isValid()) { return; }

    QModelIndex left = this->index(index.row(), ColumnName, index.parent());
    QModelIndex right = this->index(index.row(), ColumnValue, index.parent());
    emit dataChanged(left, right);
}

int PropertyModel::columnCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) { return ColumnCount; }

    Property* pp = static_cast<Property*>(parent.internalPointer());
    if (pp && !pp->children.isEmpty()) {
        return 1;
    }

    return ColumnCount;
}

QVariant PropertyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }

    const auto prop = static_cast<Property*>(index.internalPointer());
    if (!prop) { return {}; }

    QVariant result;

    switch (index.column()) {
    case ColumnName:
        if (role == Qt::DisplayRole) { return prop->name; }
        break;
    case ColumnValue:
        switch (role) {
        default:
            break;
        case Qt::DisplayRole:
            if (prop->type == PropertyType2::Enum && prop->enum_config.model) {
                int row = prop->value.toInt();
                auto row_count = prop->enum_config.model->rowCount();
                if (row < 0 && row >= row_count) { return {}; }
                auto idx = prop->enum_config.model->index(row, 0);
                if (!idx.isValid()) { return {}; }
                result = idx.data();
            } else {
                result = prop->value;
            }
            break;
        case Qt::EditRole:
            result = prop->value;
            break;
        case PropertyRole:
            result = QVariant::fromValue(prop);
            break;
        }
    }

    return result;
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) { return Qt::NoItemFlags; }

    auto prop = static_cast<Property*>(index.internalPointer());
    if (!prop) { return Qt::NoItemFlags; }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == ColumnValue && !prop->read_only) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QVariant PropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnName:
            return "Property";
        case ColumnValue:
            return "Value";
        }
    }
    return {};
}

QModelIndex PropertyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || column >= ColumnCount) {
        return QModelIndex();
    }

    Property* pp = parent.isValid()
        ? static_cast<Property*>(parent.internalPointer())
        : nullptr;

    const QList<Property*>* properties = pp
        ? &pp->children
        : &properties_;

    if (row >= properties->size()) { return QModelIndex(); }

    return createIndex(row, column, (*properties)[row]);
}

QModelIndex PropertyModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) { return QModelIndex(); }

    Property* cp = static_cast<Property*>(index.internalPointer());
    Property* pp = cp->parent;
    if (!pp) { return {}; }

    Property* gpp = pp->parent;
    const QList<Property*>* siblings = gpp ? &gpp->children : &properties_;

    return createIndex(int(siblings->indexOf(pp)), 0, pp);
}

int PropertyModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return int(properties_.size());
    }

    Property* pp = static_cast<Property*>(parent.internalPointer());
    return int(pp->children.size());
}

bool PropertyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole || index.column() != ColumnValue) { return false; }

    auto prop = static_cast<Property*>(index.internalPointer());
    if (!prop) { return false; }
    if (prop->value == value) { return false; }

    if (undo_) {
        auto cmd = new PropertyValueCommand(prop, prop->value, value, this);
        undo_->push(cmd);
        return true;
    } else {
        prop->value = value;
        if (prop->on_set) { prop->on_set(value); }
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
}

// == PropertyBrowser =========================================================
// ============================================================================

PropertyBrowser::PropertyBrowser(QWidget* parent)
    : QTreeView(parent)
{
    model_ = new PropertyModel(this);
    setModel(model_);
    setItemDelegateForColumn(PropertyModel::ColumnValue, new PropertyDelegate(this));
    setAlternatingRowColors(true);
    setUniformRowHeights(true);

    // Not really necessary since the property browser will edit on single click
    // in mousePressEvent
    setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    header()->setSectionResizeMode(QHeaderView::Stretch);

    QTimer::singleShot(0, this, [this]() {
        expandRecursively(model_->index(0, 0));
        for (int column = 0; column < model()->columnCount(); ++column) {
            resizeColumnToContents(column);
        }
    });
}

Property* PropertyBrowser::makeGroup(QString name, Property* parent)
{
    auto result = new Property;
    result->name = std::move(name);
    result->type = PropertyType2::Group;
    result->read_only = true;
    if (parent) {
        result->parent = parent;
        result->parent->children.append(result);
    }

    return result;
}

Property* PropertyBrowser::makeBoolProperty(QString name, bool value, Property* parent)
{
    auto result = new Property;
    result->name = std::move(name);
    result->value = value;
    result->type = PropertyType2::Boolean;
    result->parent = parent;
    if (result->parent) {
        result->parent->children.append(result);
    }
    return result;
}

Property* PropertyBrowser::makeDoubleProperty(QString name, double value, Property* parent)
{
    auto result = new Property;
    result->name = std::move(name);
    result->value = value;
    result->type = PropertyType2::Double;
    if (parent) {
        result->parent = parent;
        result->parent->children.append(result);
    }
    return result;
}

Property* PropertyBrowser::makeIntegerProperty(QString name, int value, Property* parent)
{
    auto result = new Property;
    result->name = std::move(name);
    result->value = value;
    result->type = PropertyType2::Integer;
    if (parent) {
        result->parent = parent;
        result->parent->children.append(result);
    }
    return result;
}

Property* PropertyBrowser::makeStringProperty(QString name, QString value, Property* parent)
{
    auto result = new Property;
    result->name = std::move(name);
    result->value = value;
    result->type = PropertyType2::String;
    if (parent) {
        result->parent = parent;
        result->parent->children.append(result);
    }
    return result;
}

Property* PropertyBrowser::makeEnumProperty(QString name, int value, QAbstractItemModel* model, Property* parent)
{
    auto result = new Property;
    result->name = std::move(name);
    result->value = value;
    result->type = PropertyType2::Enum;
    result->enum_config.model = model;
    if (parent) {
        result->parent = parent;
        result->parent->children.append(result);
    }
    return result;
}

void PropertyBrowser::addProperty(Property* prop)
{
    model_->addProperty(prop, prop->parent);
    QModelIndex propIndex = model_->indexForProperty(prop);
    if (propIndex.isValid()) {
        QModelIndex nameColIndex = model_->index(propIndex.row(), PropertyModel::ColumnName, propIndex.parent());
        expandRecursively(nameColIndex);
    }
}

void PropertyBrowser::clear()
{
    delete model_;
    model_ = new PropertyModel(this);
    setModel(model_);
    setItemDelegateForColumn(PropertyModel::ColumnValue, new PropertyDelegate(this));
}

void PropertyBrowser::expandAll()
{
    for (int i = 0; i < model()->rowCount(); ++i) {
        QModelIndex idx = model()->index(i, 0);
        expandRecursively(idx);
    }
}

PropertyModel* PropertyBrowser::model() const noexcept
{
    return model_;
}

void PropertyBrowser::setUndoStack(QUndoStack* undo)
{
    model_->setUndoStack(undo);
}

void PropertyBrowser::mousePressEvent(QMouseEvent* event)
{
    QTreeView::mousePressEvent(event);
    QModelIndex index = indexAt(event->pos());
    if (index.isValid() && index.column() == PropertyModel::ColumnValue) {
        edit(index);
    }
}
