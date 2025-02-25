#pragma once

#include "../arclighttab.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Waypoint;
}

namespace Ui {
class WaypointGeneralView;
}

class WaypointGeneralView : public ArclightTab {
    Q_OBJECT

public:
    explicit WaypointGeneralView(nw::Waypoint* obj, ArclightView* parent = nullptr);
    ~WaypointGeneralView();

private:
    Ui::WaypointGeneralView* ui;
    nw::Waypoint* obj_;
};
