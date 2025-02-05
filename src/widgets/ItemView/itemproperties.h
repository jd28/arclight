#ifndef ITEMPROPERTIES_H
#define ITEMPROPERTIES_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QUndoCommand>
#include <QWidget>

namespace nw {
struct BaseItem;
struct Item;
struct ItemProperty;
}

class ItemPropertiesModel;

namespace Ui {
class ItemProperties;
}

class ItemView;
class QListWidgetItem;
class QTableView;

// == ItemPropertiesModel =====================================================
// ============================================================================

class ItemPropertiesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    ItemPropertiesModel(nw::Item* obj, QUndoStack* undo, QObject* parent = nullptr);

    void addProperty(const nw::ItemProperty& prop);
    void addPropertyNoCmd(const nw::ItemProperty& prop);
    nw::ItemProperty getProperty(int row) const;
    void insertPropertyNoCmd(int row, const nw::ItemProperty& prop);
    void removePropertyNoCmd(int row);
    bool setDataNoCmd(const QModelIndex& index, const QVariant& value);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

private:
    nw::Item* obj_;
    QUndoStack* undo_;
};

// == ItemPropertiesEditFilter ================================================
// ============================================================================

class ItemPropertiesEditFilter : public QObject {
    Q_OBJECT
public:
    explicit ItemPropertiesEditFilter(QTableView* view, QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QTableView* m_view;
};

// == ItemPropertyDelegate ====================================================
// ============================================================================

class ItemPropertyDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ItemPropertyDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view,
        const QStyleOptionViewItem& option, const QModelIndex& index) override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

// == ItemProperties ==========================================================
// ============================================================================

class ItemProperties : public QWidget {
    Q_OBJECT

public:
    explicit ItemProperties(nw::Item* obj, ItemView* parent = nullptr);
    ~ItemProperties();

    void loadAllProperties();

public slots:
    void onBaseItemChanged(nw::BaseItem type);

private slots:
    void onItemPropertyDoubleClicked(QListWidgetItem* item);
    void deleteSelectedRows();

private:
    Ui::ItemProperties* ui;
    nw::Item* obj_;
    QUndoStack* undo_ = nullptr;
    ItemPropertiesModel* model_;
    ItemPropertyDelegate* delegate_;
};

#endif // ITEMPROPERTIES_H
