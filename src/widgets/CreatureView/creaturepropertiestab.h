#pragma once

#include "../arclighttab.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Creature;
}

namespace Ui {
class CreaturePropertiesTab;
}

class CreaturePropertiesView;

// == CreaturePropertiesTab ===================================================
// ============================================================================

class CreaturePropertiesTab : public ArclightTab {
    Q_OBJECT

public:
    explicit CreaturePropertiesTab(nw::Creature* obj, ArclightView* parent = nullptr);
    ~CreaturePropertiesTab();
    CreaturePropertiesView* properties() const noexcept;

private:
    Ui::CreaturePropertiesTab* ui;
    nw::Creature* obj_ = nullptr;
};
