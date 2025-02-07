#ifndef COLORSELECTORVIEW_H
#define COLORSELECTORVIEW_H

#include "nw/rules/items.hpp"

#include <QLabel>
#include <QWidget>

namespace nw {
struct Item;
}

namespace Ui {
class ColorSelectorView;
}

class QUndoStack;

class ColorSelectorView : public QWidget {
    Q_OBJECT

public:
    explicit ColorSelectorView(nw::Item* obj, bool has_parts, QUndoStack* undo, QWidget* parent = nullptr);
    ~ColorSelectorView();

    QPixmap getPalette(nw::ItemColors::type index) const noexcept;
    void setColor(int part, int color, int value);
    void setColorIndex(nw::ItemColors::type index);
    void setHasParts(bool has_parts);
    void setPaletteIndex(nw::ItemModelParts::type index);

private slots:
    void onClearColorClicked();
    void onColorChannelChanged(int index);
    void onPartChannelChanged(int index);
    void onValueChanged(int value);

signals:
    void colorChange(int part, int color, int value);

private:
    Ui::ColorSelectorView* ui;
    nw::Item* obj_ = nullptr;
    QUndoStack* undo_;
    QPixmap mvpal_cloth;
    QPixmap mvpal_leather;
    QPixmap mvpal_metal;
    bool has_parts_;
};

#endif // COLORSELECTORVIEW_H
