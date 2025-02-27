#include "doorgeneralview.h"
#include "ui_doorgeneralview.h"

#include "../../services/toolset/toolsetservice.h"
#include "../statictwodamodel.h"
#include "../util/itemmodels.h"
#include "../util/strings.h"
#include "../util/undocommands.h"
#include "doorproperties.h"

#include "nw/formats/Image.hpp"
#include "nw/kernel/Resources.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Door.hpp"

DoorGeneralView::DoorGeneralView(nw::Door* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::DoorGeneralView)
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
            } else {
                ui->portrait->setPixmap(QPixmap(":/images/portrait_placeholder.png"));
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

    connect(ui->properties, &DoorProperties::appearanceChanged, this, &DoorGeneralView::appearanceChanged);
}

DoorGeneralView::~DoorGeneralView()
{
    delete ui;
}
