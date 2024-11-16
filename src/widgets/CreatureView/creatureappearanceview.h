#ifndef CREATUREAPPEARANCEVIEW_H
#define CREATUREAPPEARANCEVIEW_H

#include <nw/objects/Appearance.hpp>

#include <QWidget>

namespace nw {
struct Creature;
}

namespace Ui {
class CreatureAppearanceView;
}

class CreatureAppearanceView : public QWidget {
    Q_OBJECT

public:
    explicit CreatureAppearanceView(nw::Creature* creature, QWidget* parent = nullptr);
    ~CreatureAppearanceView();

public slots:
    void onAppearanceChange(int index);
    void onOpenColorSelector();
    void onColorChanged(int color, int value);

signals:
    emit void dataChanged();

private:
    Ui::CreatureAppearanceView* ui;
    nw::Creature* creature_ = nullptr;
    QPixmap mvpal_hair;
    QPixmap mvpal_skin;

    QPixmap getPixmapIcon(nw::CreatureColors::type color) const;
};

#endif // CREATUREAPPEARANCEVIEW_H
