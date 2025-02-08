#ifndef PLACEABLEVIEW_H
#define PLACEABLEVIEW_H

#include "ArclightView.h"

#include <QWidget>

namespace nw {
struct Placeable;
struct Resource;
}

namespace Ui {
class PlaceableView;
}

class PlaceableView : public ArclightView {
    Q_OBJECT

public:
    explicit PlaceableView(nw::Resource res, QWidget* parent = nullptr);
    explicit PlaceableView(nw::Placeable* obj, QWidget* parent = nullptr);
    ~PlaceableView();

public slots:
    void loadModel();

private:
    Ui::PlaceableView* ui;
    nw::Placeable* obj_ = nullptr;
    bool owned_;
};

#endif // PLACEABLEVIEW_H
