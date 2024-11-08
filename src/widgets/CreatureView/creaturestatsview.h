#ifndef CREATURESTATSVIEW_H
#define CREATURESTATSVIEW_H

#include <QWidget>

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureStatsView;
}

class CreatureStatsView : public QWidget {
    Q_OBJECT

public:
    explicit CreatureStatsView(nw::Creature* creature, QWidget* parent = nullptr);
    ~CreatureStatsView();

    void updateAbilities();
    void updateArmorClass();
    void updateHitPoints();
    void updateSaves();

public slots:
    void onAbilityScoreChanged(int value);
    void onAcNaturalChanged(int value);
    void onMoveRateChanged(int idx);
    void onPerceptionChanged(int idx);
    void onSkillChanged(int row, int col);
    void updateAll();

private:
    Ui::CreatureStatsView* ui;
    nw::Creature* creature_ = nullptr;
};

#endif // CREATURESTATSVIEW_H
