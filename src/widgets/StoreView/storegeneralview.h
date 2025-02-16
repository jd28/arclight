#pragma once

#include "../arclighttab.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Store;
}

namespace Ui {
class StoreGeneralView;
}

class QStandardItem;
class QStandardItemModel;

// == StoreGeneralView ========================================================
// ============================================================================

class StoreGeneralView : public ArclightTab {
    Q_OBJECT

public:
    explicit StoreGeneralView(nw::Store* obj, ArclightView* parent = nullptr);
    ~StoreGeneralView();

private slots:
    void onItemChanged(QStandardItem* item);
    void onRestrictionTypeChanged(bool checked = false);

private:
    void loadProperties();

    Ui::StoreGeneralView* ui;
    nw::Store* obj_;
    QStandardItemModel* restrict_model_;
};
