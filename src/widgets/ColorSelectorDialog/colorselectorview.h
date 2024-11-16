#ifndef COLORSELECTORVIEW_H
#define COLORSELECTORVIEW_H

#include "nw/rules/items.hpp"

#include <QLabel>
#include <QWidget>

namespace Ui {
class ColorSelectorView;
}

class ColorSelectorView : public QWidget {
    Q_OBJECT

public:
    explicit ColorSelectorView(QWidget* parent = nullptr);
    ~ColorSelectorView();

    QPixmap getPalette(nw::ItemColors::type index) const noexcept;

private:
    Ui::ColorSelectorView* ui;
    QPixmap mvpal_cloth;
    QPixmap mvpal_leather;
    QPixmap mvpal_metal;
};

#endif // COLORSELECTORVIEW_H
