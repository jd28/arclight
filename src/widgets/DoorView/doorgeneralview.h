#ifndef DOORGENERALVIEW_H
#define DOORGENERALVIEW_H

#include "../arclighttab.h"

namespace nw {
struct Door;
struct Resref;
}

namespace Ui {
class DoorGeneralView;
}

class DoorGeneralView : public ArclightTab {
    Q_OBJECT

public:
    explicit DoorGeneralView(nw::Door* obj, ArclightView* parent = nullptr);
    ~DoorGeneralView();

signals:
    void appearanceChanged();

private slots:
    void onAppearanceChanged(int value);
    void onGenericChanged(int value);

private:
    Ui::DoorGeneralView* ui = nullptr;
    nw::Door* obj_ = nullptr;
};

#endif // DOORGENERALVIEW_H
