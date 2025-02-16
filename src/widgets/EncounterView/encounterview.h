#pragma once

#include "../ArclightView.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Encounter;
struct Resource;
}

namespace Ui {
class EncounterView;
}

class EncounterView : public ArclightView {
    Q_OBJECT

public:
    explicit EncounterView(nw::Resource res, QWidget* parent = nullptr);
    explicit EncounterView(nw::Encounter* obj, QWidget* parent = nullptr);
    ~EncounterView();

private:
    Ui::EncounterView* ui;
    nw::Encounter* obj_;
    bool owned_ = false;
};
