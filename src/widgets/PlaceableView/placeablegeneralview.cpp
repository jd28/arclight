#include "placeablegeneralview.h"
#include "ui_placeablegeneralview.h"

#include "../../services/toolsetservice.h"
#include "../util/strings.h"
#include "placeableproperties.h"
#include "placeableview.h"

#include "nw/formats/Image.hpp"
#include "nw/kernel/Resources.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Placeable.hpp"

PlaceableGeneralView::PlaceableGeneralView(nw::Placeable* obj, PlaceableView* parent)
    : ArclightTab(parent)
    , ui(new Ui::PlaceableGeneralView)
    , obj_{obj}
{
    ui->setupUi(this);

    if (auto portraits_2da = nw::kernel::twodas().get("portraits")) {
        auto base = portraits_2da->get<std::string>(obj_->portrait_id, "BaseResRef");
        if (base) {
            auto base_resref = fmt::format("po_{}m", *base);
            auto portrait = nw::kernel::resman().demand_in_order(nw::Resref(base_resref),
                {nw::ResourceType::dds, nw::ResourceType::tga});

            if (portrait.bytes.size()) {
                auto img = nw::Image(std::move(portrait));
                QImage qi(img.data(), img.width(), img.height(),
                    img.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
                if (qi.height() > 128 || qi.width() > 128) {
                    qi = qi.scaled(128, 128, Qt::KeepAspectRatio);
                }

                // These are pre-flipped
                if (img.is_bio_dds()) { qi.mirror(); }

                QRect rect(0, 0, 64, 100); // This is specific to medium portraits
                qi = qi.copy(rect);
                ui->portrait->setPixmap(QPixmap::fromImage(qi));
            }
        }
    } else {
        LOG_F(ERROR, "Failed to load portraits.2da");
    }

    ui->name->setLocString(obj->common.name);
    ui->tag->setText(to_qstring(obj->tag().view()));
    ui->resref->setText(to_qstring(obj->common.resref.view()));
    ui->resref->setEnabled(obj->common.resref.empty());
    ui->properties->setObject(obj_);
    ui->properties->setUndoStack(undoStack());

    auto app_idx = mapSourceRowToProxyRow(toolset().placeable_model.get(),
        toolset().placeable_filter.get(), obj_->appearance);
    ui->appearance->setModel(toolset().placeable_filter.get());
    ui->appearance->setCurrentIndex(app_idx);

    connect(ui->appearance, &QComboBox::currentIndexChanged, this, &PlaceableGeneralView::onAppearanceChanged);
    connect(this, &PlaceableGeneralView::modificationChanged, parent, &PlaceableView::onModificationChanged);
    connect(ui->properties, &PlaceableProperties::hasInventoryChanged, parent, &PlaceableView::onHasInvetoryChanged);
}

PlaceableGeneralView::~PlaceableGeneralView()
{
    delete ui;
}

void PlaceableGeneralView::onAppearanceChanged(int value)
{
    obj_->appearance = static_cast<uint32_t>(ui->appearance->itemData(value, Qt::UserRole + 1).toInt());
    emit appearanceChanged();
}
