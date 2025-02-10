#pragma once

#include <QWidget>

class ArclightTab;
class QUndoStack;

/// Arclight View is an abstraction for any widget that is placed in the main window tab.
class ArclightView : public QWidget {
    Q_OBJECT

public:
    ArclightView(QWidget* parent = nullptr);
    virtual ~ArclightView() = default;

    /// Adds a sub-tab to for the view to track
    void addTab(ArclightTab* tab);

    /// Get is modified.
    bool modified() const noexcept;

    /// Is view read only
    bool readOnly() const noexcept;

public slots:
    void onModificationChanged(bool modified);

signals:
    void activateUndoStack(QUndoStack*);
    void modificationChanged(bool modified);

private:
    QList<ArclightTab*> tabs_;
    bool read_only_ = false;
    bool modified_ = false;
};
