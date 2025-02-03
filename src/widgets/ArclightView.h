#ifndef ARCLIGHTVIEW_H
#define ARCLIGHTVIEW_H

#include <QWidget>

class QUndoStack;

/// Arclight View is an abstraction for any widget that is placed in the main window tab.
class ArclightView : public QWidget {
    Q_OBJECT

public:
    ArclightView(QWidget* parent = nullptr);
    virtual ~ArclightView() = default;

    /// Is view read only
    bool readOnly() const noexcept;

    /// Gets undo stack
    QUndoStack* undoStack() const noexcept;

signals:
    void activateUndoStack(QUndoStack*);

protected:
    void focusInEvent(QFocusEvent* event) override;

private:
    bool read_only_ = false;
    QUndoStack* undo_stack_;
};

#endif // ARCLIGHTVIEW_H
