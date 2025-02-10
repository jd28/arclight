#ifndef PLACEABLEPROPERTIES_H
#define PLACEABLEPROPERTIES_H

#include "../propertybrowser.h"

#include <QMap>
#include <QWidget>

namespace nw {
struct Placeable;
}

class QCompleter;

class PlaceableProperties : public PropertyBrowser {
    Q_OBJECT
public:
    explicit PlaceableProperties(QWidget* parent = nullptr);
    void setObject(nw::Placeable* obj);

signals:
    void hasInventoryChanged(bool value);

private:
    Property* lock_locked_prop_ = nullptr;
    Property* lock_lockable_prop_ = nullptr;
    Property* lock_remove_key_prop_ = nullptr;
    Property* lock_key_required_prop_ = nullptr;
    Property* lock_key_name_prop_ = nullptr;
    Property* lock_lock_dc_prop_ = nullptr;
    Property* lock_unlock_dc_prop_ = nullptr;

    Property* trap_type_ = nullptr;
    Property* trap_is_trapped_ = nullptr;
    Property* trap_detectable_ = nullptr;
    Property* trap_detect_dc_ = nullptr;
    Property* trap_disarmable_ = nullptr;
    Property* trap_disarm_dc_ = nullptr;
    Property* trap_one_shot_ = nullptr;

    nw::Placeable* obj_ = nullptr;
    QCompleter* script_completer_ = nullptr;

    void basicsLoad();
    void conversationLoad();
    void locksLoad();
    void locksUpdate();
    void savesLoad();
    void scriptsLoad();
    void trapsLoad();
    void trapsUpdate();
};

#endif // PLACEABLEPROPERTIES_H
