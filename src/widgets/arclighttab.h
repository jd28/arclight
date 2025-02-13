#pragma once

#include <QWidget>

class ArclightView;
class QUndoStack;

class ArclightTab : public QWidget {
    Q_OBJECT
public:
    explicit ArclightTab(ArclightView* parent = nullptr);
    ~ArclightTab();

    bool modified() const noexcept;
    QUndoStack* undoStack() const noexcept;

signals:
    void activateUndoStack(QUndoStack*);
    void modificationChanged(bool modified);

private:
    QUndoStack* undo_;
    bool modified_ = false;
};
