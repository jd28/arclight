#pragma once

#include "../ArclightView.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Resource;
struct Sound;
}

namespace Ui {
class SoundView;
}

// == SoundView ===============================================================
// ============================================================================

class SoundView : public ArclightView {
    Q_OBJECT

public:
    explicit SoundView(nw::Resource res, QWidget* parent = nullptr);
    explicit SoundView(nw::Sound* obj, QWidget* parent = nullptr);
    ~SoundView();

private:
    Ui::SoundView* ui;
    nw::Sound* obj_;
    bool owned_ = false;
};
