#include "itemgeneralview.h"
#include "ui_itemgeneralview.h"

#include "../../services/rulesetmodels.h"
#include "../../services/toolsetservice.h"
#include "../ColorSelectorDialog/colorselectordialog.h"
#include "../ColorSelectorDialog/colorselectorview.h"
#include "../proxymodels.h"
#include "../util/objects.h"
#include "../util/strings.h"
#include "itemsimplemodelselectordialog.h"

#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/objects/Item.hpp"
#include "nw/profiles/nwn1/constants.hpp"

#include <QShortcut>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTimer>
#include <QUndoStack>

// == ItemPropView ============================================================
// ============================================================================

ItemPropView::ItemPropView(QWidget* parent)
    : PropertyBrowser{parent}
{
    connect(this, &ItemPropView::baseItemChanged, this, &ItemPropView::loadAppearanceProperties);
}

void ItemPropView::setObject(nw::Item* obj)
{
    if (obj_) { return; }
    obj_ = obj;

    loadBasicProperties();
    loadAppearanceProperties(obj_->baseitem);
    expandAll();
}

void ItemPropView::loadAppearanceProperties(nw::BaseItem type)
{
    auto new_appearance = makeGroup("Appearance");

    auto bi_info = nw::kernel::rules().baseitems.get(type);
    if (!bi_info || !bi_info->valid()) { return; }

    auto find_index = [](QStandardItemModel* model, int value) {
        for (int i = 0; i < model->rowCount(); ++i) {
            if (model->item(i)->data().toInt() == value) {
                return i;
            }
        }
        return -1;
    };

    switch (bi_info->model_type) {
    case nw::ItemModelType::simple:
    case nw::ItemModelType::layered: {
        QStandardItemModel* model = nullptr;

        if (bi_info->model_type == nw::ItemModelType::simple) {
            auto needle = fmt::format("i{}", bi_info->item_class.view());
            model = toolset().get_simple_models(needle);
        } else {
            if (obj_->baseitem == nwn1::base_item_cloak) {
                model = toolset().cloak_model.get();
            } else {
                model = toolset().get_layered_models(bi_info->item_class.view());
            }
        }
        int index = find_index(model, obj_->model_parts[nw::ItemModelParts::model1]);
        auto p = makeEnumProperty("Model", index, model, new_appearance);
        p->on_set = [this, model](const QVariant& value) {
            auto idx = model->index(value.toInt(), 0);
            int mv = model->data(idx, Qt::UserRole + 1).toInt();
            obj_->model_parts[nw::ItemModelParts::model1] = uint16_t(mv);
            emit updateModel();
        };

        if (bi_info->model_type == nw::ItemModelType::simple) {
            p->dialog = [this, bi_info](Property* prop, QWidget* parent) -> QLabel* {
                ItemSimpleModelSelectorDialog dlg(obj_->baseitem, prop->value.toInt(), this);
                int w = std::max(64, bi_info->inventory_slot_size.first * 32);
                int h = std::max(64, bi_info->inventory_slot_size.second * 32);
                dlg.selector()->setIconSize(QSize(w, h));
                if (dlg.exec() == QDialog::Accepted) {
                    QVariant value = dlg.selector()->currentIndex();
                    auto* editor = new QLabel(dlg.selector()->currentText(), parent);
                    editor->setProperty("value", value);
                    return editor;
                }
                return nullptr;
            };
        }
    } break;
    case nw::ItemModelType::composite: {
        auto models = toolset().get_composite_models(bi_info->item_class.view());
        if (models.top_model->rowCount() == 0) {
            auto needle = fmt::format("i{}", bi_info->item_class.view());
            models = toolset().get_composite_models(needle, false);
        }

        // clang-format off
#define ADD_COMPOSITE_PART(name, part_number, top_model, color_model) \
        do { \
                auto color_proxy = new VariantListFilterProxyModel(obj_->model_parts[part_number] / 10, Qt::UserRole + 2, this); \
                color_proxy->setSourceModel(color_model); \
                auto top = makeGroup(name, new_appearance); \
                int index = find_index(top_model, obj_->model_parts[part_number] / 10); \
                auto m = makeEnumProperty("Model", index, top_model, top); \
                m->on_set = [this, model = top_model, color_proxy](const QVariant& value) { \
                          auto idx = model->index(value.toInt(), 0); \
                          int mv = model->data(idx, Qt::UserRole + 1).toInt() * 10; \
                          auto cv = obj_->model_parts[part_number] % 10; \
                          obj_->model_parts[part_number] = uint16_t(mv + cv); \
                          color_proxy->setTargetValue(model->data(idx, Qt::UserRole + 1).toInt()); \
                          emit updateModel(); \
                  }; \
                index = find_index(color_model, obj_->model_parts[part_number] % 10); \
                index = mapSourceRowToProxyRow(color_model, color_proxy, index); \
                auto c = makeEnumProperty("Color", index, color_proxy, top); \
                c->on_set = [this, model = color_proxy](const QVariant& value) { \
                          auto idx = model->index(value.toInt(), 0); \
                          auto mv = (obj_->model_parts[part_number] / 10) * 10; \
                          int cv = idx.data(Qt::UserRole + 1).toInt(); \
                          obj_->model_parts[part_number] = uint16_t(mv + cv); \
                          emit updateModel(); /* clazy:skip */ \
                  }; \
        } while (0)
        // clang-format on

        ADD_COMPOSITE_PART("Top", nw::ItemModelParts::model3, models.top_model, models.top_color);
        ADD_COMPOSITE_PART("Middle", nw::ItemModelParts::model2, models.middle_model, models.middle_color);
        ADD_COMPOSITE_PART("Bottom", nw::ItemModelParts::model1, models.bottom_model, models.bottom_color);

        // clang-format off
#undef ADD_COMPOSITE_PART
        // clang-format on

    } break;
    case nw::ItemModelType::armor: {
        // clang-format off
#define ADD_ARMOR_PART(name, part_number, part_model) \
        do { \
                int index = find_index(part_model, obj_->model_parts[part_number]); \
                Property* property = makeEnumProperty(name, index, part_model, new_appearance); \
                property->on_set = [this, model = part_model](const QVariant& value) { \
                          auto idx = model->index(value.toInt(), 0); \
                          int mv = model->data(idx, Qt::UserRole + 1).toInt(); \
                          obj_->model_parts[part_number] = uint16_t(mv); \
                          emit updateModel(); /* clazy:skip */ \
                  }; \
        } while (0)
        // clang-format on

        ADD_ARMOR_PART("Belt", nw::ItemModelParts::armor_belt, toolset().parts_belt.get());
        ADD_ARMOR_PART("Bicep, Left", nw::ItemModelParts::armor_lbicep, toolset().parts_bicep.get());
        ADD_ARMOR_PART("Bicep, Right", nw::ItemModelParts::armor_rbicep, toolset().parts_bicep.get());
        ADD_ARMOR_PART("Foot, Left", nw::ItemModelParts::armor_lfoot, toolset().parts_foot.get());
        ADD_ARMOR_PART("Foot, Right", nw::ItemModelParts::armor_rfoot, toolset().parts_foot.get());
        ADD_ARMOR_PART("Forearm, Left", nw::ItemModelParts::armor_lfarm, toolset().parts_forearm.get());
        ADD_ARMOR_PART("Forearm, Right", nw::ItemModelParts::armor_rfarm, toolset().parts_forearm.get());
        ADD_ARMOR_PART("Hand, Left", nw::ItemModelParts::armor_lhand, toolset().parts_hand.get());
        ADD_ARMOR_PART("Hand, Right", nw::ItemModelParts::armor_rhand, toolset().parts_hand.get());
        ADD_ARMOR_PART("Neck", nw::ItemModelParts::armor_neck, toolset().parts_neck.get());
        ADD_ARMOR_PART("Pelvis", nw::ItemModelParts::armor_pelvis, toolset().parts_pelvis.get());
        ADD_ARMOR_PART("Robe", nw::ItemModelParts::armor_robe, toolset().parts_robe.get());
        ADD_ARMOR_PART("Shin, Left", nw::ItemModelParts::armor_lshin, toolset().parts_shin.get());
        ADD_ARMOR_PART("Shin, Right", nw::ItemModelParts::armor_rshin, toolset().parts_shin.get());
        ADD_ARMOR_PART("Shoulder, Left", nw::ItemModelParts::armor_lshoul, toolset().parts_shoulder.get());
        ADD_ARMOR_PART("Shoulder, Right", nw::ItemModelParts::armor_rshoul, toolset().parts_shoulder.get());
        ADD_ARMOR_PART("Thigh, Left", nw::ItemModelParts::armor_lthigh, toolset().parts_legs.get());
        ADD_ARMOR_PART("Thigh, Right", nw::ItemModelParts::armor_rthigh, toolset().parts_legs.get());
        ADD_ARMOR_PART("Torso", nw::ItemModelParts::armor_torso, toolset().parts_chest.get());

        // clang-format off
#undef ADD_ARMOR_PART
        // clang-format on
    } break;
    }

    if (!appearance_group_) {
        addProperty(new_appearance);
    } else {
        model()->removeProperty(appearance_group_);
        addProperty(new_appearance);
    }
    appearance_group_ = new_appearance;
}

void ItemPropView::loadBasicProperties()
{
    auto bi_info = nw::kernel::rules().baseitems.get(obj_->baseitem);
    if (!bi_info || !bi_info->valid()) { return; }

    auto grp_basic = makeGroup("Basic");

    QModelIndex sidx = toolset().baseitem_model->index(*obj_->baseitem, 0, {});
    QModelIndex pidx = toolset().baseitem_filter->mapFromSource(sidx);

    auto base = makeEnumProperty("Base Item Type", pidx.row(), toolset().baseitem_filter.get(), grp_basic);
    base->on_set = [this](const QVariant& value) {
        auto index = value.toInt();
        int row = mapProxyRowToSourceRow(toolset().baseitem_filter.get(), index);
        if (row == -1) { return; }
        obj_->baseitem = nw::BaseItem::make(row);
        obj_->model_parts.fill(0);
        obj_->model_colors.fill(0);
        for (auto& colors : obj_->part_colors) {
            colors.fill(255);
        }
        emit baseItemChanged(obj_->baseitem);
    };

    auto additional_cost = makeIntegerProperty("Additional Cost", obj_->additional_cost, grp_basic);
    additional_cost->on_set = [this](const QVariant& value) {
        obj_->additional_cost = value.toInt();
    };

    auto plot = makeBoolProperty("Plot", obj_->plot, grp_basic);
    plot->on_set = [this](const QVariant& value) {
        obj_->plot = value.toBool();
    };

    auto stack_size = makeIntegerProperty("Stack Size", obj_->stacksize, grp_basic);
    stack_size->int_config.min = 1;
    stack_size->int_config.max = bi_info->stack_limit;
    stack_size->on_set = [this](const QVariant& value) {
        obj_->stacksize = uint16_t(value.toInt());
    };

    addProperty(grp_basic);
}

// == ItemGeneralView =========================================================
// ============================================================================

ItemGeneralView::ItemGeneralView(nw::Item* obj, ItemView* parent)
    : QWidget(parent)
    , ui(new Ui::ItemGeneralView)
    , obj_{obj}
    , undo_{new QUndoStack(this)}
{
    ui->setupUi(this);

    mvpal_cloth_ = QPixmap(":/resources/images/mvpal_cloth.png");
    mvpal_leather_ = QPixmap(":/resources/images/mvpal_leather.png");
    mvpal_metal_ = QPixmap(":/resources/images/mvpal_armor01.png");

    ui->name->setLocString(obj_->common.name);
    ui->tag->setText(to_qstring(obj_->tag().view()));
    ui->resref->setText(to_qstring(obj_->common.resref.view()));

    loadIcon();

    ui->properties->setObject(obj_);
    ui->properties->setUndoStack(undo_);
    connect(ui->properties, &ItemPropView::updateModel, this, &ItemGeneralView::onUpdateModel);

    QShortcut* us = new QShortcut(QKeySequence::Undo, this);
    QShortcut* rs = new QShortcut(QKeySequence::Redo, this);
    connect(us, &QShortcut::activated, undo_, &QUndoStack::undo);
    connect(rs, &QShortcut::activated, undo_, &QUndoStack::redo);

    auto bi_info = nw::kernel::rules().baseitems.get(obj_->baseitem);
    if (!bi_info || !bi_info->valid()) { return; }

    bool has_colors = bi_info->model_type == nw::ItemModelType::layered
        || bi_info->model_type == nw::ItemModelType::armor;

    if (has_colors) {
        ui->metal_1->setIcon(getPalletteColorIcon(nw::ItemColors::metal1));
        ui->metal_2->setIcon(getPalletteColorIcon(nw::ItemColors::metal2));
        ui->leather_1->setIcon(getPalletteColorIcon(nw::ItemColors::leather1));
        ui->leather_2->setIcon(getPalletteColorIcon(nw::ItemColors::leather2));
        ui->cloth_1->setIcon(getPalletteColorIcon(nw::ItemColors::cloth1));
        ui->cloth_2->setIcon(getPalletteColorIcon(nw::ItemColors::cloth2));
    }

    ui->colors->setHidden(!has_colors);

    connect(ui->properties, &ItemPropView::baseItemChanged, this, &ItemGeneralView::onBaseItemChanged);

    connect(ui->metal_1, &QPushButton::clicked, this, &ItemGeneralView::onOpenColorSelector);
    connect(ui->metal_2, &QPushButton::clicked, this, &ItemGeneralView::onOpenColorSelector);
    connect(ui->leather_1, &QPushButton::clicked, this, &ItemGeneralView::onOpenColorSelector);
    connect(ui->leather_2, &QPushButton::clicked, this, &ItemGeneralView::onOpenColorSelector);
    connect(ui->cloth_1, &QPushButton::clicked, this, &ItemGeneralView::onOpenColorSelector);
    connect(ui->cloth_2, &QPushButton::clicked, this, &ItemGeneralView::onOpenColorSelector);
}

ItemGeneralView::~ItemGeneralView()
{
    delete ui;
}

QPixmap ItemGeneralView::getPalletteColorIcon(nw::ItemColors::type color) const
{
    auto selected = obj_->model_colors[color];

    int col = selected % 16;
    int row = selected / 16;
    QRect rect(col * 32, row * 32, 32, 32);

    switch (color) {
    case nw::ItemColors::cloth1:
    case nw::ItemColors::cloth2:
        return mvpal_cloth_.copy(rect).scaled(16, 16);
    case nw::ItemColors::leather1:
    case nw::ItemColors::leather2:
        return mvpal_leather_.copy(rect).scaled(16, 16);
    case nw::ItemColors::metal1:
    case nw::ItemColors::metal2:
        return mvpal_metal_.copy(rect).scaled(16, 16);
    }

    return {};
}

void ItemGeneralView::loadIcon()
{
    auto img = item_to_image(obj_, false);
    if (!img.isNull()) {
        ui->icon->setPixmap(QPixmap::fromImage(img));
        ui->icon->setToolTip(to_qstring(nw::kernel::strings().get(obj_->common.name)));
    }
}

void ItemGeneralView::onBaseItemChanged(nw::BaseItem type)
{
    auto bi_info = nw::kernel::rules().baseitems.get(obj_->baseitem);
    if (!bi_info || !bi_info->valid()) { return; }

    bool has_colors = bi_info->model_type == nw::ItemModelType::layered
        || bi_info->model_type == nw::ItemModelType::armor;

    ui->colors->setHidden(has_colors);
    emit baseItemChanged(type);
}

void ItemGeneralView::onColorChanged(int part, int color, int value)
{
    if (part == -1) {
        obj_->model_colors[color] = uint8_t(value);
        ui->metal_1->setIcon(getPalletteColorIcon(nw::ItemColors::metal1));
        ui->metal_2->setIcon(getPalletteColorIcon(nw::ItemColors::metal2));
        ui->leather_1->setIcon(getPalletteColorIcon(nw::ItemColors::leather1));
        ui->leather_2->setIcon(getPalletteColorIcon(nw::ItemColors::leather2));
        ui->cloth_1->setIcon(getPalletteColorIcon(nw::ItemColors::cloth1));
        ui->cloth_2->setIcon(getPalletteColorIcon(nw::ItemColors::cloth2));
    } else {
        obj_->part_colors[part][color] = uint8_t(value);
    }

    loadIcon();
}

void ItemGeneralView::onOpenColorSelector()
{
    auto bi_info = nw::kernel::rules().baseitems.get(obj_->baseitem);
    if (!bi_info || !bi_info->valid()) { return; }

    if (!color_dialog_) {
        color_dialog_.reset(new ColorSelectionDialog(obj_, bi_info->model_type == nw::ItemModelType::armor, this));
        color_dialog_->setWindowModality(Qt::NonModal);
        connect(color_dialog_->selector(), &ColorSelectorView::colorChange, this, &ItemGeneralView::onColorChanged);
        QTimer::singleShot(0, this, [this]() {
            color_dialog_->setFixedSize(color_dialog_->size());
        });
    }
    color_dialog_->selector()->setHasParts(bi_info->model_type == nw::ItemModelType::armor);

    overlay_ = new QWidget(parentWidget()->parentWidget());
    overlay_->setStyleSheet("background-color: rgba(0, 0, 0, 10);");
    overlay_->setGeometry(parentWidget()->parentWidget()->rect());
    overlay_->show();

    connect(color_dialog_.get(), &QDialog::finished, this, [this](int result) {
        Q_UNUSED(result);
        cleanupOverlayAndDialog();
    });

    connect(color_dialog_->selector(), &ColorSelectorView::colorChange, this, &ItemGeneralView::onColorChanged);
    if (sender()->objectName() == "metal_1") {
        color_dialog_->selector()->setColorIndex(nw::ItemColors::metal1);
    } else if (sender()->objectName() == "metal_2") {
        color_dialog_->selector()->setColorIndex(nw::ItemColors::metal2);
    } else if (sender()->objectName() == "leather_1") {
        color_dialog_->selector()->setColorIndex(nw::ItemColors::leather1);
    } else if (sender()->objectName() == "leather_2") {
        color_dialog_->selector()->setColorIndex(nw::ItemColors::leather2);
    } else if (sender()->objectName() == "cloth_1") {
        color_dialog_->selector()->setColorIndex(nw::ItemColors::cloth1);
    } else if (sender()->objectName() == "cloth_2") {
        color_dialog_->selector()->setColorIndex(nw::ItemColors::cloth2);
    }
    color_dialog_->show();
    color_dialog_->raise();
    color_dialog_->activateWindow();
}

void ItemGeneralView::onUpdateModel()
{
    loadIcon();
}

void ItemGeneralView::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    cleanupOverlayAndDialog();
}

void ItemGeneralView::cleanupOverlayAndDialog()
{
    if (overlay_) {
        overlay_->deleteLater();
        overlay_ = nullptr;
    }
    if (color_dialog_) {
        color_dialog_->close();
        color_dialog_.reset();
    }
}
