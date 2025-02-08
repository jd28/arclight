#ifndef DOORVIEW_H
#define DOORVIEW_H

#include "../ArclightView.h"

#include <QWidget>

namespace nw {
struct Door;
struct Resource;
}

namespace Ui {
class DoorView;
}

class DoorView : public ArclightView {
    Q_OBJECT

public:
    explicit DoorView(nw::Resource res, QWidget* parent = nullptr);
    explicit DoorView(nw::Door* obj, QWidget* parent = nullptr);
    ~DoorView();

public slots:
    void loadModel();

private:
    Ui::DoorView* ui;
    nw::Door* obj_ = nullptr;
    bool owned_ = false;
};

#endif // DOORVIEW_H
