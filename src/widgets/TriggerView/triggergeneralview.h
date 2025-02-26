#pragma once

#include "../arclighttab.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Resource;
struct Trigger;
}

namespace Ui {
class TriggerGeneralView;
}

class Property;

// == TriggerGeneralView ======================================================
// ============================================================================

class TriggerGeneralView : public ArclightTab {
    Q_OBJECT

public:
    explicit TriggerGeneralView(nw::Trigger* obj, ArclightView* parent = nullptr);
    ~TriggerGeneralView();

private:
    void scriptsLoad();
    void transitionLoad();
    void trapsLoad();
    void trapsUpdate();

    Ui::TriggerGeneralView* ui;
    nw::Trigger* obj_ = nullptr;

    Property* trap_type_ = nullptr;
    Property* trap_is_trapped_ = nullptr;
    Property* trap_detectable_ = nullptr;
    Property* trap_detect_dc_ = nullptr;
    Property* trap_disarmable_ = nullptr;
    Property* trap_disarm_dc_ = nullptr;
    Property* trap_one_shot_ = nullptr;
};
