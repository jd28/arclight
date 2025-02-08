#ifndef AREAVIEW_H
#define AREAVIEW_H

#include "../ArclightView.h"

#include <memory>

class BasicTileArea;

namespace nw {
struct Area;
struct Resource;
}

namespace Ui {
class AreaView;
}

class AreaView : public ArclightView {
    Q_OBJECT

public:
    explicit AreaView(nw::Resource area, QWidget* parent = nullptr);
    ~AreaView();

    void loadModel();

private:
    Ui::AreaView* ui;
    nw::Area* obj_ = nullptr;
    std::unique_ptr<BasicTileArea> area_model_;
};

#endif // AREAVIEW_H
