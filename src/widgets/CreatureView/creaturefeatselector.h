#ifndef CREATUREFEATSELECTOR_H
#define CREATUREFEATSELECTOR_H

#include <QWidget>

class CreatureFeatSelectorModel;
class CreatureFeatSelectorSortFilterProxy;

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureFeatSelector;
}

class CreatureFeatSelector : public QWidget {
    Q_OBJECT

public:
    explicit CreatureFeatSelector(nw::Creature* creature, QWidget* parent = nullptr);
    ~CreatureFeatSelector();

    CreatureFeatSelectorModel* model() const;

private:
    Ui::CreatureFeatSelector* ui;
    CreatureFeatSelectorModel* model_ = nullptr;
    CreatureFeatSelectorSortFilterProxy* filter_ = nullptr;
    nw::Creature* creature_ = nullptr;
};

#endif // CREATUREFEATSELECTOR_H
