#include "doorgeneralview.h"
#include "ui_doorgeneralview.h"

#include "../util/strings.h"
#include "doorproperties.h"

#include "nw/formats/Image.hpp"
#include "nw/kernel/Resources.hpp"
#include "nw/kernel/Strings.hpp"
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

    auto doortypes = nw::kernel::twodas().get("doortypes");
    if (doortypes) {
        int name;
        std::string model;
        int added = 0;
        for (size_t i = 0; i < doortypes->rows(); ++i) {
            if (!doortypes->get_to(i, "StringRefGame", name)) { continue; }
            ui->appearance->addItem(to_qstring(nw::kernel::strings().get(uint32_t(name))), int(i));
            ++i;
            if (obj_->appearance == i) {
                ui->appearance->setCurrentIndex(added);
            }
            ++added;
        }
    } else {
        throw std::runtime_error("[door] failed to load doortypes.2da");
    }
    // Not sorting the appearance combo due to special "Use Generic Doors"

    auto genericdoors = nw::kernel::twodas().get("genericdoors");
    if (genericdoors) {
        int name;
        std::string model;
        int added = 0;
        for (size_t i = 0; i < genericdoors->rows(); ++i) {
            if (!genericdoors->get_to(i, "Name", name)) { continue; }
            ui->generic->addItem(to_qstring(nw::kernel::strings().get(uint32_t(name))), int(i));
            ++i;
            if (obj_->generic_type == i) {
                ui->generic->setCurrentIndex(added);
            }
            ++added;
        }
    } else {
        throw std::runtime_error("[door] failed to load genericdoors.2da");
    }
    ui->generic->model()->sort(0);
    ui->generic->setEnabled(obj_->appearance == 0);

    connect(ui->appearance, &QComboBox::currentIndexChanged, this, &DoorGeneralView::onAppearanceChanged);
    connect(ui->generic, &QComboBox::currentIndexChanged, this, &DoorGeneralView::onGenericChanged);
}

DoorGeneralView::~DoorGeneralView()
{
    delete ui;
}

void DoorGeneralView::onAppearanceChanged(int value)
{
    obj_->appearance = static_cast<uint32_t>(ui->appearance->itemData(value).toInt());
    ui->generic->setEnabled(obj_->appearance == 0);
    emit appearanceChanged();
}

void DoorGeneralView::onGenericChanged(int value)
{
    obj_->generic_type = static_cast<uint32_t>(ui->generic->itemData(value).toInt());
    emit appearanceChanged();
}
