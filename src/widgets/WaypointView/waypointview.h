#pragma once

#include "../ArclightView.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace Ui {
class WaypointView;
}

namespace nw {
struct Resource;
struct Waypoint;
}

// == WaypointView ============================================================
// ============================================================================

class WaypointView : public ArclightView {
    Q_OBJECT

public:
    explicit WaypointView(nw::Resource res, QWidget* parent = nullptr);
    explicit WaypointView(nw::Waypoint* obj, QWidget* parent = nullptr);
    ~WaypointView();

private:
    Ui::WaypointView* ui;
    nw::Waypoint* obj_ = nullptr;
    bool owned_ = false;
};
