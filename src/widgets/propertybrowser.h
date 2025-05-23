#ifndef PROPERTYBROWSER_H
#define PROPERTYBROWSER_H

#include <QAbstractItemModel>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QTreeView>

class QCompleter;
class QSortFilterProxyModel;
class QUndoStack;
class QLabel;

class AutoCommitComboBox : public QComboBox {
    Q_OBJECT
public:
    AutoCommitComboBox(QWidget* parent = nullptr)
        : QComboBox(parent)
    {
        connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AutoCommitComboBox::onIndexChanged);
    }

    void setInitialized() { init_ = true; }

private slots:
    void onIndexChanged()
    {
        if (init_) {
            if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent()->parent())) {
                QAbstractItemModel* model = view->model();
                QModelIndex index = view->currentIndex();
                model->setData(index, currentIndex(), Qt::EditRole);
            }
        }
    }

private:
    bool init_ = false;
};

class AutoCommitSpinBox : public QSpinBox {
    Q_OBJECT
public:
    AutoCommitSpinBox(QWidget* parent = nullptr)
        : QSpinBox(parent)
    {
        connect(this, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AutoCommitSpinBox::onValueChanged);
    }
    void setInitialized() { init_ = true; }

private slots:
    void onValueChanged()
    {
        if (init_) {
            if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent()->parent())) {
                QAbstractItemModel* model = view->model();
                QModelIndex index = view->currentIndex();
                model->setData(index, value(), Qt::EditRole);
            }
        }
    }

private:
    bool init_ = false;
};

class AutoCommitDoubleSpinBox : public QDoubleSpinBox {
    Q_OBJECT
public:
    AutoCommitDoubleSpinBox(QWidget* parent = nullptr)
        : QDoubleSpinBox(parent)
    {
        connect(this, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AutoCommitDoubleSpinBox::onValueChanged);
    }
    void setInitialized() { init_ = true; }

private slots:
    void onValueChanged()
    {
        if (init_) {
            if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent()->parent())) {
                QAbstractItemModel* model = view->model();
                QModelIndex index = view->currentIndex();
                model->setData(index, value(), Qt::EditRole);
            }
        }
    }

private:
    bool init_ = false;
};

class AutoCommitLineEdit : public QLineEdit {
    Q_OBJECT
public:
    AutoCommitLineEdit(QWidget* parent = nullptr)
        : QLineEdit(parent)
    {
        connect(this, &QLineEdit::textChanged,
            this, &AutoCommitLineEdit::onTextChanged);
    }
    void setInitialized() { init_ = true; }

private slots:
    void onTextChanged()
    {
        if (init_) {
            if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent()->parent())) {
                QAbstractItemModel* model = view->model();
                QModelIndex index = view->currentIndex();
                model->setData(index, text(), Qt::EditRole);
            }
        }
    }

private:
    bool init_ = false;
};

class AutoCommitCheckBox : public QCheckBox {
    Q_OBJECT
public:
    AutoCommitCheckBox(QWidget* parent = nullptr)
        : QCheckBox(parent)
    {
        connect(this, &QCheckBox::clicked,
            this, &AutoCommitCheckBox::onClicked);
    }
    void setInitialized() { init_ = true; }

private slots:
    void onClicked()
    {
        if (init_) {
            if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent()->parent())) {
                QAbstractItemModel* model = view->model();
                QModelIndex index = view->currentIndex();
                model->setData(index, isChecked(), Qt::EditRole);
            }
        }
    }

private:
    bool init_ = false;
};

enum struct PropertyType2 {
    Group,
    Boolean,
    Enum,
    Integer,
    String,
    Double,
};

struct EnumPropConfig {
    QAbstractItemModel* model = nullptr;
};

struct StringPropConfig {
    QCompleter* completer = nullptr;
    QValidator* validator = nullptr;
};

struct IntPropConfig {
    QVariant min;
    QVariant max;
    QVariant step;
};

struct DoublePropConfig {
    QVariant min;
    QVariant max;
    QVariant step;
};

class Property {
public:
    Property() = default;
    ~Property();

    Property(const Property&) = delete;
    Property(Property&&) = delete;
    Property& operator=(const Property&) = delete;
    Property& operator=(Property&&) = delete;

    void setEditable(bool val);
    void setReadOnly(bool val);
    void removeFromParent();

    QString name;
    QVariant value;
    PropertyType2 type;
    bool read_only = false;

    DoublePropConfig double_config;
    EnumPropConfig enum_config;
    IntPropConfig int_config;
    StringPropConfig string_config;

    std::function<QLabel*(Property* prop, QWidget*)> dialog;
    std::function<void(const QVariant&)> on_set;
    std::function<bool(const QVariant&)> validator;

    QList<Property*> children;
    Property* parent = nullptr;
};

// == PropertyDelegate ========================================================
// ============================================================================

class PropertyDelegate : public QStyledItemDelegate {
public:
    PropertyDelegate(QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

// == PropertyModel ===========================================================
// ============================================================================

class PropertyModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum CustomRoles {
        PropertyRole = Qt::UserRole + 1
    };

    enum PropertyColumns {
        ColumnName = 0,
        ColumnValue,
        ColumnCount
    };

    PropertyModel(QObject* parent = nullptr);
    ~PropertyModel();

    void addProperty(Property* prop, Property* parent = nullptr);
    void deleteProperty(Property* prop);
    QModelIndex indexForProperty(Property* prop) const;
    void removeProperty(Property* prop);
    void replaceProperty(Property* old, Property* replacement);
    void setUndoStack(QUndoStack* undo);
    void updateReadOnly(Property* prop);

    // QAbstractItemModel overrides
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

signals:
    void propertyChanged(Property* prop);

private:
    QList<Property*> properties_;
    QUndoStack* undo_ = nullptr;
};

class PropertyBrowser : public QTreeView {
public:
    PropertyBrowser(QWidget* parent = nullptr);

    Property* makeGroup(QString name, Property* parent = nullptr);
    Property* makeBoolProperty(QString name, bool value, Property* parent = nullptr);
    Property* makeDoubleProperty(QString name, double value, Property* parent = nullptr);
    Property* makeEnumProperty(QString name, int value, QAbstractItemModel* model, Property* parent = nullptr);
    Property* makeIntegerProperty(QString name, int value, Property* parent = nullptr);
    Property* makeStringProperty(QString name, QString value, Property* parent = nullptr);

    void addProperty(Property* prop);
    void clear();
    PropertyModel* model() const noexcept;
    void setUndoStack(QUndoStack* undo);

protected:
    void mousePressEvent(QMouseEvent* event) override;

    PropertyModel* model_;
};

#endif // PROPERTYBROWSER_H
