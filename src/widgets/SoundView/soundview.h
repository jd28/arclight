#pragma once

#include "../ArclightView.h"

namespace Ui {
class SoundView;
}

class SoundView : public ArclightView {
    Q_OBJECT

public:
    explicit SoundView(QWidget* parent = nullptr);
    ~SoundView();

private:
    Ui::SoundView* ui;
};
