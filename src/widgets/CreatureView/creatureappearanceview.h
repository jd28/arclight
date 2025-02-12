#ifndef CREATUREAPPEARANCEVIEW_H
#define CREATUREAPPEARANCEVIEW_H

#include "../arclighttab.h"

#include <nw/objects/Appearance.hpp>

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureAppearanceView;
}

class CreatureAppearanceView : public ArclightTab {
    Q_OBJECT

public:
    explicit CreatureAppearanceView(nw::Creature* obj, ArclightView* parent = nullptr);
    ~CreatureAppearanceView();

public slots:
    void onColorChanged(int color, int value);
    void onOpenColorSelector();

private:
    Ui::CreatureAppearanceView* ui;
    nw::Creature* obj_ = nullptr;
    bool is_dynamic_ = false;
    QPixmap mvpal_hair;
    QPixmap mvpal_skin;

    QPixmap getPixmapIcon(nw::CreatureColors::type color) const;
    void updateEnabled();
};

#endif // CREATUREAPPEARANCEVIEW_H
