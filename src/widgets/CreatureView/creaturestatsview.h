#pragma once

#include <QWidget>

// Deprecated

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureStatsView;
}

// == CreatureStatsView =======================================================
// ============================================================================

class CreatureStatsView : public QWidget {
    Q_OBJECT

public:
    explicit CreatureStatsView(nw::Creature* creature, QWidget* parent = nullptr);
    ~CreatureStatsView();

    void updateAbilities();
    void updateArmorClass();
    void updateHitPoints();
    void updateSaves();
    void updateSkills();

public slots:
    void onAcNaturalChanged(int value);
    void onHitpointBaseChanged(int value);
    void updateAll();

private:
    Ui::CreatureStatsView* ui;
    nw::Creature* creature_ = nullptr;
};
