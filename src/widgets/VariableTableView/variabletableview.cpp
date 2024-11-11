#include "variabletableview.h"
#include "ui_variabletableview.h"

#include "../comboboxdelegate.h"

#include "nw/objects/LocalData.hpp"

#include "../util/strings.h"
#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include <QLineEdit>
#include <QRegularExpressionValidator>

inline bool isNumber(const QString& text)
{
    bool ok;
    text.toInt(&ok);
    if (ok) return true;
    text.toFloat(&ok);
    return ok;
}

// == VariableTableModel ======================================================
// ============================================================================

VariableTableModel::VariableTableModel(nw::LocalData* locals, QObject* parent)
    : QAbstractTableModel(parent)
    , locals_{locals}
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
    if (!index.isValid())
        return QAbstractTableModel::flags(index);

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

const QList<VarTableItem*>& VariableTableModel::modified_variables() const
{
    return qlocals_;
}

bool VariableTableModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    Q_UNUSED(role);
    if (!idx.isValid()) { return false; }
    auto ptr = static_cast<VarTableItem*>(idx.internalPointer());

    if (idx.column() == 0) {
        ptr->name = value.toString();
    } else if (idx.column() == 1) {
        auto str = value.toString();
        if (str == "int") {
            if (ptr->type == nw::LocalVarType::integer) { return false; }
            ptr->type = nw::LocalVarType::integer;
            ptr->data = 0;
            emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
            return true;
        } else if (str == "string") {
            if (ptr->type == nw::LocalVarType::string) { return false; }
            ptr->type = nw::LocalVarType::string;
            ptr->data = "";
            emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
            return true;
        } else if (str == "float") {
            if (ptr->type == nw::LocalVarType::float_) { return false; }
            ptr->type = nw::LocalVarType::float_;
            ptr->data = 0.0;
            emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
            return true;
        }
    } else if (idx.column() == 2) {
        ptr->data = value;
        emit dataChanged(idx, idx);
        return true;
    }

    return false;
}

void VariableTableModel::addRow()
{
    int rows = rowCount(QModelIndex());
    beginInsertRows(QModelIndex(), rows, rows);
    qlocals_.push_back(new VarTableItem{"<variable name>", nw::LocalVarType::integer, 0});
    endInsertRows();
}

void VariableTableModel::deleteRow(const QModelIndex& index)
{
    auto ptr = static_cast<VarTableItem*>(index.internalPointer());

    beginRemoveRows(QModelIndex(), index.row(), index.row());
    qlocals_.removeOne(ptr);
    delete ptr;
    endRemoveRows();
}

// == VariableTableView =======================================================
// ============================================================================

VariableTableView::VariableTableView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::VariableTableView)
{
    ui->setupUi(this);
    QStringList types;
    types << "int"
          << "string"
          << "float";
    ui->tableView->setItemDelegateForColumn(1, new ComboBoxDelegate(types, this));
    ui->add->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_plus, Qt::green));
    ui->delete_2->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_minus, Qt::red));
    connect(ui->delete_2, &QPushButton::clicked, this, &VariableTableView::onDeleteClicked);
    connect(ui->add, &QPushButton::clicked, this, &VariableTableView::onAddClicked);
}

VariableTableView::~VariableTableView()
{
    delete ui;
}

void VariableTableView::setLocals(nw::LocalData* locals)
{
    model_ = new VariableTableModel(locals, this);
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
