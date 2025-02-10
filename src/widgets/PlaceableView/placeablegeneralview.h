#pragma once

#include "../arclighttab.h"

namespace nw {
struct Placeable;
struct Resref;
}

namespace Ui {
class PlaceableGeneralView;
}

class PlaceableView;
class QUndoStack;

class PlaceableGeneralView : public ArclightTab {
    Q_OBJECT

public:
    explicit PlaceableGeneralView(nw::Placeable* obj, PlaceableView* parent = nullptr);
    ~PlaceableGeneralView();

signals:
    void appearanceChanged();

private slots:
    void onAppearanceChanged(int value);

private:
    Ui::PlaceableGeneralView* ui = nullptr;
    nw::Placeable* obj_ = nullptr;
};
