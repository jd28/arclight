#pragma once

#include "../ArclightView.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Resource;
struct Trigger;
}

namespace Ui {
class TriggerView;
}

// == TriggerView =============================================================
// ============================================================================

class TriggerView : public ArclightView {
    Q_OBJECT

public:
    explicit TriggerView(nw::Resource res, QWidget* parent = nullptr);
    explicit TriggerView(nw::Trigger* obj, QWidget* parent = nullptr);
    ~TriggerView();

private:
    Ui::TriggerView* ui;
    nw::Trigger* obj_;
    bool owned_ = false;
};
