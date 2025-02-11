#pragma once

#include "arclighttab.h"

namespace Ui {
class LoadscreensView;
}

class QItemSelection;

class LoadscreensView : public ArclightTab {
    Q_OBJECT

public:
    explicit LoadscreensView(int value, ArclightView* parent = nullptr);
    ~LoadscreensView();

    void setIndex(int value);

private slots:
    void onCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

signals:
    void valueChanged(int value);

private:
    Ui::LoadscreensView* ui;
    int index_ = -1;
};
