#ifndef CREATUREVIEW_H
#define CREATUREVIEW_H

#include "../ArclightView.h"

#include <QWidget>

class CreatureFeatSelector;

namespace nw {
struct Creature;
struct Resource;
}

namespace Ui {
class CreatureView;
}

class CreatureView : public ArclightView {
    Q_OBJECT

public:
    explicit CreatureView(nw::Resource res, QWidget* parent = nullptr);
    explicit CreatureView(nw::Creature* obj, QWidget* parent = nullptr);
    ~CreatureView();

    void loadCreature(nw::Creature* creature);

public slots:
    void onUpdateModel();

private:
    Ui::CreatureView* ui;
    nw::Creature* obj_ = nullptr;
    bool owned_ = false;
    CreatureFeatSelector* feat_selector_ = nullptr;
};

#endif // CREATUREVIEW_H
