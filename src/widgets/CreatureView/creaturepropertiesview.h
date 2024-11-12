#ifndef CREATUREPROPERTIESVIEW_H
#define CREATUREPROPERTIESVIEW_H

#include "propertiesview.h"

namespace nw {
struct Creature;
} // namespace nw

class QCompleter;
class QtProperty;

class CreaturePropertiesView : public PropertiesView {
    Q_OBJECT

public:
    explicit CreaturePropertiesView(QWidget* parent = nullptr);
    ~CreaturePropertiesView();

    void setCreature(nw::Creature* obj);

public slots:
    void onPropertyChanged(QtProperty* prop);

signals:
    void updateStats();

private:
    nw::Creature* obj_ = nullptr;
    QCompleter* script_completer_ = nullptr;
    QMap<QtProperty*, std::function<void(QtProperty*)>> prop_func_map_;

    void loadProperties();
    void loadAbilities();
    void loadInterface();
    void loadSaves();
    void loadScripts();
    void loadSkills();
    void loadAdvanced();
    void loadBasic();
};

#endif // CREATUREPROPERTIESVIEW_H
