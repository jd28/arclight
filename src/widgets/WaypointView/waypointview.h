#pragma once

#include "../ArclightView.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace Ui {
class WaypointView;
}

// == WaypointView ============================================================
// ============================================================================

class WaypointView : public ArclightView {
    Q_OBJECT

public:
    explicit WaypointView(QWidget* parent = nullptr);
    ~WaypointView();

private:
    Ui::WaypointView* ui;
};
