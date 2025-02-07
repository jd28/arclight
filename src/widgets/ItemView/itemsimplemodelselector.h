#pragma once

#include "nw/rules/items.hpp"

#include <QWidget>

namespace Ui {
class ItemSimpleModelSelector;
}

class ItemSimpleModelSelector : public QWidget {
    Q_OBJECT

public:
    explicit ItemSimpleModelSelector(nw::BaseItem type, int current, QWidget* parent = nullptr);
    ~ItemSimpleModelSelector();
    QVariant value() const;
    int currentIndex() const;
    QString currentText() const;
    void setIconSize(QSize size);

private:
    Ui::ItemSimpleModelSelector* ui;
    nw::BaseItem type_;
    int current_ = 0;
};
