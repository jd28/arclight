#ifndef CREATUREVIEW_H
#define CREATUREVIEW_H

#include "../ArclightView.h"

#include <QWidget>

class CreatureFeatSelector;

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureView;
}

class CreatureView : public ArclightView {
    Q_OBJECT

public:
    explicit CreatureView(nw::Creature* creature, QWidget* parent = nullptr);
    ~CreatureView();

    void loadCreature(nw::Creature* creature);

public slots:
    void onModified();

private:
    Ui::CreatureView* ui;
    nw::Creature* obj_ = nullptr;
    CreatureFeatSelector* feat_selector_ = nullptr;
};

#endif // CREATUREVIEW_H
