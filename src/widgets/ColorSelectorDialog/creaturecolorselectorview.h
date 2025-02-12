#ifndef CREATURECOLORSELECTORVIEW_H
#define CREATURECOLORSELECTORVIEW_H

#include "nw/objects/Appearance.hpp"

#include <QWidget>

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureColorSelector;
}

class QUndoStack;

class CreatureColorSelectorView : public QWidget {
    Q_OBJECT

public:
    explicit CreatureColorSelectorView(nw::Creature* obj, QUndoStack* undo = nullptr, QWidget* parent = nullptr);
    ~CreatureColorSelectorView();

    void setIndex(nw::CreatureColors::type index);
    void setColor(int color, int value);

public slots:
    void onColorChanelChanged(int index);
    void onValueChanged(int value);

signals:
    void colorChange(int color, int value);

private:
    Ui::CreatureColorSelector* ui;
    nw::Creature* obj_;
    QUndoStack* undo_;
    QPixmap mvpal_hair;
    QPixmap mvpal_skin;
};

#endif // CREATURECOLORSELECTORVIEW_H
