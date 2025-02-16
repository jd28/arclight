#pragma once

#include "../InventoryView/inventoryview.h"
#include "../arclighttab.h"

namespace nw {
struct Store;
}

namespace Ui {
class StoreInventoryView;
}

class StoreInventoryView : public ArclightTab {
    Q_OBJECT

public:
    explicit StoreInventoryView(nw::Store* obj, ArclightView* parent = nullptr);
    ~StoreInventoryView();

private:
    Ui::StoreInventoryView* ui;
    nw::Store* obj_ = nullptr;
    QList<InventoryView*> tabs_;
};
