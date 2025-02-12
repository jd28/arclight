#ifndef CREATUREPARTSVIEW_H
#define CREATUREPARTSVIEW_H

#include "../propertybrowser.h"

namespace nw {
struct Creature;
}

class CreaturePartsView : public PropertyBrowser {
    Q_OBJECT
public:
    CreaturePartsView(QWidget* parent = nullptr);

    void loadProperties();
    void setCreature(nw::Creature* obj);

signals:
    void updateModel();

private:
    nw::Creature* obj_ = nullptr;
};

#endif // CREATUREPARTSVIEW_H
