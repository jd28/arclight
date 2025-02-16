#pragma once

#include "../ArclightView.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Resource;
struct Store;
}

namespace Ui {
class StoreView;
}

// == StoreView ===============================================================
// ============================================================================

class StoreView : public ArclightView {
    Q_OBJECT

public:
    explicit StoreView(nw::Resource res, QWidget* parent);
    explicit StoreView(nw::Store* obj, QWidget* parent = nullptr);
    ~StoreView();

private:
    Ui::StoreView* ui;
    nw::Store* obj_;
    bool owned_ = false;
};
