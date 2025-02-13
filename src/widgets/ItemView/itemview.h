#ifndef ITEMVIEW_H
#define ITEMVIEW_H

#include "../ArclightView.h"

namespace nw {
struct BaseItem;
struct Item;
struct Resource;
}

namespace Ui {
class ItemView;
}

class InventoryView;

class QUndoStack;


// == ItemView ================================================================
// ============================================================================

class ItemView : public ArclightView {
    Q_OBJECT

public:
    explicit ItemView(nw::Resource res, QWidget* parent = nullptr);
    explicit ItemView(nw::Item* obj, QWidget* parent = nullptr);
    ~ItemView();

public slots:
    void onBaseItemChanged(nw::BaseItem bi);

private slots:
    void onTabChanged(int index);

private:
    Ui::ItemView* ui;
    nw::Item* obj_;
    bool owned_ = false;
    InventoryView* inventory_;
};

#endif // ITEMVIEW_H
