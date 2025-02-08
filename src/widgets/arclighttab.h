#ifndef ARCLIGHTTAB_H
#define ARCLIGHTTAB_H

#include <QWidget>

class ArclightView;
class QUndoStack;

class ArclightTab : public QWidget {
    Q_OBJECT
public:
    explicit ArclightTab(ArclightView* parent = nullptr);

    QUndoStack* undoStack() const noexcept;

signals:
    void activateUndoStack(QUndoStack*);
    void modified();

private:
    QUndoStack* undo_stack_;
};

#endif // ARCLIGHTTAB_H
