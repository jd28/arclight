#include "loadscreensview.h"
#include "ui_loadscreensview.h"

#include "../services/toolsetservice.h"
#include "util/itemmodels.h"
#include "util/undocommands.h"

#include <QApplication>
#include <QIcon>
#include <QListView>
#include <QPainter>
#include <QResizeEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTimer>

LoadscreensView::LoadscreensView(int value, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::LoadscreensView)
{
    ui->setupUi(this);
    auto model = toolset().loadscreens_model.get();
    ui->view->setModel(model);

    QTimer::singleShot(0, this, [this]() {
        QObject::connect(ui->view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &LoadscreensView::onCurrentChanged);
    });

    index_ = findStandardItemIndex(model, value);
    if (index_ == -1) { return; }

    QModelIndex index = model->index(index_, 0);
    ui->view->setCurrentIndex(index);
    ui->view->scrollTo(index);
}

LoadscreensView::~LoadscreensView()
{
    delete ui;
}

void LoadscreensView::setIndex(int value)
{
    QSignalBlocker blocker(ui->view->selectionModel());

    index_ = value;
    if (value == -1) {
        ui->view->clearSelection();
        ui->view->setCurrentIndex(QModelIndex());
        ui->view->scrollToTop();
    } else {
        auto index = ui->view->model()->index(index_, 0);
        ui->view->setCurrentIndex(index);
        ui->view->scrollTo(index, QAbstractItemView::PositionAtCenter);
        emit valueChanged(index.data(Qt::UserRole + 1).toInt());
    }
}

void LoadscreensView::onCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    if (current.isValid() && index_ != current.row()) {
        auto undo = new VariantUndoCommand(index_, current.row(), [this](const QVariant& value) {
            setIndex(value.toInt());
        });
        undoStack()->push(undo);
    }
}
