#include "variabletableview.h"
#include "ui_variabletableview.h"

#include "../comboboxdelegate.h"

#include "nw/objects/LocalData.hpp"

#include "../util/strings.h"
#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include <QLineEdit>
#include <QMouseEvent>
#include <QRegularExpressionValidator>
#include <QShortcut>
#include <QUndoCommand>

inline bool isNumber(const QString& text)
{
    bool ok;
    text.toInt(&ok);
    if (ok) return true;
    text.toFloat(&ok);
    return ok;
}

class VarTableAddCommand : public QUndoCommand {
public:
    VarTableAddCommand(VariableTableModel* model, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , model_(model)
        , row_(model_->rowCount())
    {
        setText("Add Variable");
    }

    void undo() override
    {
        model_->removeRow(row_);
    }

    void redo() override
    {
        model_->insertRow(row_, new VarTableItem{"<variable name>", nw::LocalVarType::integer, 0});
    }

private:
    VariableTableModel* model_;
    int row_;
};
class VarTableDeleteCommand : public QUndoCommand {
public:
    VarTableDeleteCommand(VariableTableModel* model, int row, VarTableItem* item, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , model_(model)
        , row_(row)
        , item_{*item}
    {
        setText("Delete Variable");
    }

    void undo() override
    {
        model_->insertRow(row_, new VarTableItem{item_});
    }

    void redo() override
    {
        if (row_ < 0 || row_ >= model_->rowCount()) {
            qWarning() << "Redo failed: Invalid delete command!";
            return;
        }
        model_->removeRow(row_);
    }

private:
    VariableTableModel* model_;
    int row_;
    VarTableItem item_; // Store a copy, no need for manual memory management
};

class VarTableModifyCommand : public QUndoCommand {
public:
    VarTableModifyCommand(VariableTableModel* model, const QModelIndex& index,
        const QVariant& oldValue, const QVariant& newValue,
        QUndoCommand* parent = nullptr)
        : QUndoCommand(parent)
        , model_(model)
        , row_(index.row())
        , column_(index.column())
        , oldValue_(oldValue)
        , newValue_(newValue)
    {
        setText("Modify Variable");
    }

    void undo() override
    {
        doSetData(oldValue_);
    }

    void redo() override
    {
        doSetData(newValue_);
    }

private:
    void doSetData(const QVariant& value)
    {
        auto index = model_->index(row_, column_);
        auto ptr = static_cast<VarTableItem*>(index.internalPointer());

        if (index.column() == 0) {
            ptr->name = value.toString();
            emit model_->dataChanged(index, index);
        } else if (index.column() == 1) {
            auto str = value.toString();
            if (str == "int") {
                ptr->type = nw::LocalVarType::integer;
                ptr->data = 0;
            } else if (str == "string") {
                ptr->type = nw::LocalVarType::string;
                ptr->data = "";
            } else if (str == "float") {
                ptr->type = nw::LocalVarType::float_;
                ptr->data = 0.0;
            }
            emit model_->dataChanged(model_->index(0, 0),
                model_->index(model_->rowCount() - 1, model_->columnCount() - 1));
        } else if (index.column() == 2) {
            ptr->data = value;
            emit model_->dataChanged(index, index);
        }
    }

    VariableTableModel* model_;
    int row_;
    int column_;
    QVariant oldValue_;
    QVariant newValue_;
};

// == VariableTableModel ======================================================
// ============================================================================

VariableTableModel::VariableTableModel(nw::LocalData* locals, QUndoStack* undo, QObject* parent)
    : QAbstractTableModel(parent)
    , locals_{locals}
    , undo_{undo}
{
    for (const auto& localvar : *locals_) {
        VarTableItem* loc = new VarTableItem;

        loc->name = to_qstring(localvar.first);
        if (localvar.second.flags.test(nw::LocalVarType::integer)) {
            loc->type = nw::LocalVarType::integer;
            loc->data = localvar.second.integer;
            qlocals_.push_back(loc);
        }

        if (localvar.second.flags.test(nw::LocalVarType::string)) {
            loc->type = nw::LocalVarType::string;
            loc->data = to_qstring(localvar.second.string);
            qlocals_.push_back(loc);
        }

        if (localvar.second.flags.test(nw::LocalVarType::float_)) {
            loc->type = nw::LocalVarType::float_;
            loc->data = localvar.second.float_;
            qlocals_.push_back(loc);
        }
    }
}

VariableTableModel::~VariableTableModel()
{
    qDeleteAll(qlocals_);
}

int VariableTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(qlocals_.size());
}

int VariableTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant VariableTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }
    auto ptr = static_cast<VarTableItem*>(index.internalPointer());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        default:
            return {};
        case 0:
            return qlocals_[index.row()]->name;
        case 1:
            switch (qlocals_[index.row()]->type) {
            default:
                return {};
            case nw::LocalVarType::integer:
                return "int";
            case nw::LocalVarType::string:
                return "string";
            case nw::LocalVarType::float_:
                return "float";
            }

        case 2:
            return qlocals_[index.row()]->data;
        }
    } else if (index.column() == 2) {
        if (ptr->type == nw::LocalVarType::string && isNumber(ptr->data.toString())) {
            if (role == Qt::DecorationRole) {
                return ZFontIcon::icon(Fa6::FAMILY, Fa6::SOLID, Fa6::fa_triangle_exclamation, Qt::yellow);
            } else if (role == Qt::ToolTipRole) {
                return "String value is convertable to a number";
            }
        }
    } else if (index.column() == 0) {
        int dupes = 0;
        for (auto ql : qlocals_) {
            if (ql->name == ptr->name && ql->type == ptr->type) {
                ++dupes;
            }
        }
        if (dupes > 1) {
            if (role == Qt::DecorationRole) {
                return ZFontIcon::icon(Fa6::FAMILY, Fa6::SOLID, Fa6::fa_circle_exclamation, Qt::red);
            } else if (role == Qt::ToolTipRole) {
                return "Duplicate local variable entry";
            }
        }
    }
    return {};
}

QVariant VariableTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    default:
        return {};
    case 0:
        return "Name";
    case 1:
        return "Type";
    case 2:
        return "Value";
    }
}

QModelIndex VariableTableModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column, qlocals_[row]);
}

Qt::ItemFlags VariableTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) { return QAbstractTableModel::flags(index); }
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

const QList<VarTableItem*>& VariableTableModel::modified_variables() const
{
    return qlocals_;
}

bool VariableTableModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid() || role != Qt::EditRole) { return false; }

    auto oldValue = data(idx, Qt::EditRole);
    if (oldValue == value) { return false; }
    undo_->push(new VarTableModifyCommand(this, idx, oldValue, value));
    LOG_F(INFO, "pushing undo");
    return true;
}

void VariableTableModel::addRow()
{
    undo_->push(new VarTableAddCommand(this));
    LOG_F(INFO, "pushing undo");
}

void VariableTableModel::insertRow(int row, VarTableItem* item)
{
    qlocals_.insert(row, item);
    beginInsertRows(QModelIndex(), row, row);
    endInsertRows();
}

void VariableTableModel::removeRow(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    auto item = qlocals_.takeAt(row);
    delete item;
    endRemoveRows();
}

void VariableTableModel::deleteRow(const QModelIndex& index)
{
    if (!index.isValid()) return;

    auto ptr = static_cast<VarTableItem*>(index.internalPointer());
    undo_->push(new VarTableDeleteCommand(this, index.row(), ptr));
    LOG_F(INFO, "pushing undo");
}

// == SinglClickEditFilter ====================================================
// ============================================================================

SinglClickEditFilter::SinglClickEditFilter(QTableView* view, QObject* parent)
    : QObject(parent)
    , m_view(view)
{
}

bool SinglClickEditFilter::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = m_view->indexAt(mouseEvent->pos());
        if (index.isValid()) {
            m_view->edit(index);
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

// == VariableTableView =======================================================
// ============================================================================

VariableTableView::VariableTableView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::VariableTableView)
    , undo_{new QUndoStack(this)}
{
    ui->setupUi(this);
    QStringList types;
    types << "int"
          << "string"
          << "float";
    ui->tableView->setItemDelegateForColumn(1, new ComboBoxDelegate(types, this));
    auto filter = new SinglClickEditFilter(ui->tableView, this);
    ui->tableView->viewport()->installEventFilter(filter);
    ui->add->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_plus, Qt::green));
    ui->delete_2->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_minus, Qt::red));
    connect(ui->delete_2, &QPushButton::clicked, this, &VariableTableView::onDeleteClicked);
    connect(ui->add, &QPushButton::clicked, this, &VariableTableView::onAddClicked);

    QShortcut* us = new QShortcut(QKeySequence::Undo, this);
    QShortcut* rs = new QShortcut(QKeySequence::Redo, this);
    connect(us, &QShortcut::activated, undo_, &QUndoStack::undo);
    connect(rs, &QShortcut::activated, undo_, &QUndoStack::redo);
}

VariableTableView::~VariableTableView()
{
    delete ui;
}

void VariableTableView::setLocals(nw::LocalData* locals)
{
    model_ = new VariableTableModel(locals, undo_, this);
    model_->sort(0);
    ui->tableView->setModel(model_);
    ui->delete_2->setDisabled(model_->rowCount() == 0);

    QHeaderView* header = ui->tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
}

void VariableTableView::onAddClicked()
{
    model_->addRow();
    ui->delete_2->setDisabled(model_->rowCount() == 0);
    auto index = model_->index(model_->rowCount() - 1, 0, QModelIndex());
    ui->tableView->setCurrentIndex(index);
    ui->tableView->edit(index);
}

void VariableTableView::onDeleteClicked()
{
    auto selection = ui->tableView->selectionModel()->currentIndex();
    model_->deleteRow(selection);
    ui->delete_2->setDisabled(model_->rowCount() == 0);
}
