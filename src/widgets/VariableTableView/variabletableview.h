#ifndef VARIABLETABLEVIEW_H
#define VARIABLETABLEVIEW_H

#include "nw/objects/LocalData.hpp"

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QWidget>

namespace Ui {
class VariableTableView;
}

class QUndoStack;
class QTableView;

// == VariableTableProxy ======================================================
// ============================================================================

class VariableTableProxy : public QSortFilterProxyModel {
};

// == VariableTableModel ======================================================
// ============================================================================

// Note: in the case of float, the variant will contain a double
struct VarTableItem {
    QString name;
    uint32_t type;
    QVariant data;
};

class VariableTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    VariableTableModel(nw::LocalData* locals, QUndoStack* undo, QObject* parent = nullptr);
    ~VariableTableModel();

    void addRow();
    void deleteRow(const QModelIndex& index);
    void insertRow(int row, VarTableItem* item);
    void removeRow(int row);

    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    const QList<VarTableItem*>& modified_variables() const;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;

private:
    nw::LocalData* locals_ = nullptr;
    QList<VarTableItem*> qlocals_;
    QUndoStack* undo_;
};

// == SinglClickEditFilter ====================================================
// ============================================================================

class SinglClickEditFilter : public QObject {
    Q_OBJECT
public:
    explicit SinglClickEditFilter(QTableView* view, QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QTableView* m_view;
};

// == VariableTableView =======================================================
// ============================================================================

class VariableTableView : public QWidget {
    Q_OBJECT

public:
    explicit VariableTableView(QWidget* parent = nullptr);
    ~VariableTableView();

    VariableTableModel* model() const { return model_; };
    void setLocals(nw::LocalData* locals);

public slots:
    void onAddClicked();
    void onDeleteClicked();

private:
    Ui::VariableTableView* ui;
    VariableTableModel* model_ = nullptr;
    QUndoStack* undo_;
};

#endif // VARIABLETABLEVIEW_H
