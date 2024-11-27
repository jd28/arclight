#ifndef CREATUREPARTSVIEW_H
#define CREATUREPARTSVIEW_H

#include "../propertiesview.h"

namespace nw {
struct Creature;
}

class CreaturePartsView : public PropertiesView {
    Q_OBJECT
public:
    CreaturePartsView(QWidget* parent = nullptr);

    void clear();
    void loadProperties();
    void setCreature(nw::Creature* obj);
    void setEnabled(bool enabled);

public slots:
    void onPropertyChanged(QtProperty* prop);

signals:
    void updateModel();

private:
    nw::Creature* obj_ = nullptr;
    QMap<QtProperty*, std::function<void(QtProperty*)>> prop_func_map_;
};

#endif // CREATUREPARTSVIEW_H
