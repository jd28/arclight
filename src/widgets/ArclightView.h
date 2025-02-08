#pragma once

#include <QWidget>

class QUndoStack;

/// Arclight View is an abstraction for any widget that is placed in the main window tab.
class ArclightView : public QWidget {
    Q_OBJECT

public:
    ArclightView(QWidget* parent = nullptr);
    virtual ~ArclightView() = default;

    /// Get is modified.
    bool isModified() const noexcept;

    /// Is view read only
    bool readOnly() const noexcept;

    /// Set is modified.
    void setModified(bool value);

signals:
    void activateUndoStack(QUndoStack*);
    void modified();

private:
    bool read_only_ = false;
    bool modified_ = false;
};
