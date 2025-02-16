#pragma once

#include "../arclighttab.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Encounter;
}

namespace Ui {
class EncounterPropsView;
}

class EncounterCreatureModel;
class Property;

// == EncounterPropsView ======================================================
// ============================================================================

class EncounterPropsView : public ArclightTab {
    Q_OBJECT

public:
    explicit EncounterPropsView(nw::Encounter* obj, ArclightView* parent);
    ~EncounterPropsView();

    bool eventFilter(QObject* object, QEvent* event) override;

private:
    void loadProperties();

    Ui::EncounterPropsView* ui;
    nw::Encounter* obj_;
    EncounterCreatureModel* creatures_;
    Property* respawn_count_;
};
