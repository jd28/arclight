#pragma once

#include "../arclighttab.h"

namespace nw {
struct Class;
struct Creature;
}

namespace Ui {
class CreatureCharSheetView;
}

class CreatureView;

class CreatureCharSheetView : public ArclightTab {
    Q_OBJECT

public:
    explicit CreatureCharSheetView(nw::Creature* obj, CreatureView* parent = nullptr);
    ~CreatureCharSheetView();

    void loadCreature(nw::Creature* obj);
    void loadPortrait(nw::Creature* obj);

signals:
    void classAdded(nw::Class class_);
    void classRemoved(nw::Class class_);

private slots:
    void onClassChanged(int index);
    void onClassDeleteButtonClicked();
    void onClassLevelChanged(int value);

private:
    Ui::CreatureCharSheetView* ui;
    nw::Creature* obj_ = nullptr;
};
