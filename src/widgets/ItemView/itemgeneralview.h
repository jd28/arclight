#ifndef ITEMGENERALVIEW_H
#define ITEMGENERALVIEW_H

#include "../propertybrowser.h"

#include "nw/rules/items.hpp"

#include <QWidget>

namespace nw {
struct Item;
}

namespace Ui {
class ItemGeneralView;
}

class ColorSelectionDialog;
class ItemView;
class QUndoStack;

// == ItemPropView ============================================================
// ============================================================================

class ItemPropView : public PropertyBrowser {
    Q_OBJECT
public:
    explicit ItemPropView(QWidget* parent = nullptr);

    void setObject(nw::Item* obj);

signals:
    void baseItemChanged(nw::BaseItem type);
    void updateModel();

private slots:
    void loadAppearanceProperties(nw::BaseItem type);

private:
    void loadBasicProperties();

    nw::Item* obj_ = nullptr;
    Property* appearance_group_ = nullptr;
    Property* baseitems_ = nullptr;
};

class ItemGeneralView : public QWidget {
    Q_OBJECT

public:
    explicit ItemGeneralView(nw::Item* obj, ItemView* parent = nullptr);
    ~ItemGeneralView();

    QPixmap getPalletteColorIcon(nw::ItemColors::type color) const;
    void loadIcon();

public slots:
    void onBaseItemChanged(nw::BaseItem type);
    void onColorChanged(int part, int color, int value);
    void onOpenColorSelector();
    void onUpdateModel();

signals:
    void baseItemChanged(nw::BaseItem type);

protected:
    void hideEvent(QHideEvent* event) override;

private:
    void cleanupOverlayAndDialog();

    Ui::ItemGeneralView* ui;
    nw::Item* obj_;
    QUndoStack* undo_;
    QPixmap mvpal_metal_;
    QPixmap mvpal_leather_;
    QPixmap mvpal_cloth_;
    std::unique_ptr<ColorSelectionDialog> color_dialog_;
    QWidget* overlay_ = nullptr;
};

#endif // ITEMGENERALVIEW_H
