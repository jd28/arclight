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
    void loadClasses();

void setupClassWidgets(nw::Creature* creature);

public slots:
    void onClassAddClicked(bool checked = false);
    void onClassChanged(int index);
    void onClassDelClicked(bool checked = false);
    void onClassLevelChanged(int value);
    void onDataChanged();

private:
    Ui::CreatureView* ui;
    nw::Creature* creature_ = nullptr;
    CreatureFeatSelector* feat_selector_ = nullptr;
    int current_classes_ = 0;
};

#endif // CREATUREVIEW_H
