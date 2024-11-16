#ifndef CREATURECOLORSELECTORVIEW_H
#define CREATURECOLORSELECTORVIEW_H

#include "nw/objects/Appearance.hpp"

#include <QWidget>

namespace Ui {
class CreatureColorSelector;
}

class CreatureColorSelectorView : public QWidget {
    Q_OBJECT

public:
    explicit CreatureColorSelectorView(QWidget* parent = nullptr);
    ~CreatureColorSelectorView();

    void setIndex(nw::CreatureColors::type index);
    void setColors(std::array<uint8_t, 4> colors);

public slots:
    void onColorChanelChanged(int index);
    void onValueChanged(int value);

signals:
    void colorChange(int color, int value);

private:
    Ui::CreatureColorSelector* ui;
    std::array<uint8_t, 4> colors_;
    QPixmap mvpal_hair;
    QPixmap mvpal_skin;
};

#endif // CREATURECOLORSELECTORVIEW_H
