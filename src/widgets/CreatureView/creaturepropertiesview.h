#pragma once

#include "../propertybrowser.h"

namespace nw {
struct Creature;
} // namespace nw

class QCompleter;

class CreaturePropertiesView : public PropertyBrowser {
    Q_OBJECT

public:
    explicit CreaturePropertiesView(QWidget* parent = nullptr);
    ~CreaturePropertiesView();

    void setCreature(nw::Creature* obj);

signals:
    void reloadStats();

private:
    nw::Creature* obj_ = nullptr;
    QCompleter* script_completer_ = nullptr;

    void loadProperties();
    void loadAbilities();
    void loadInterface();
    void loadSaves();
    void loadScripts();
    void loadSkills();
    void loadAdvanced();
    void loadBasic();
};
