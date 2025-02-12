#ifndef CREATRURECOLORSELECTORDIALOG_H
#define CREATRURECOLORSELECTORDIALOG_H

#include "nw/objects/Appearance.hpp"

namespace nw {
struct Creature;
}

class CreatureColorSelectorView;
class QUndoStack;

#include <QDialog>

class CreatureColorSelectionDialog : public QDialog {
    Q_OBJECT

public:
    CreatureColorSelectionDialog(nw::Creature* obj, QUndoStack* undo, QWidget* parent = nullptr);

    CreatureColorSelectorView* selector() const noexcept { return selector_; }

private:
    CreatureColorSelectorView* selector_;
    nw::CreatureColors::type index_;
};

#endif // CREATURECOLORSELECTORDIALOG_H
