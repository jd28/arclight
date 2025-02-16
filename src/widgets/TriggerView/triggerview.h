#pragma once

#include "../ArclightView.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace Ui {
class TriggerView;
}

// == TriggerView =============================================================
// ============================================================================

class TriggerView : public ArclightView {
    Q_OBJECT

public:
    explicit TriggerView(QWidget* parent = nullptr);
    ~TriggerView();

private:
    Ui::TriggerView* ui;
};
