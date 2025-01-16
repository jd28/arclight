#include "placeablegeneralview.h"
#include "ui_placeablegeneralview.h"

#include "../VariableTableView/vartabledialog.h"
#include "../util/strings.h"
#include "placeableproperties.h"

#include "nw/formats/Image.hpp"
#include "nw/kernel/Resources.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Placeable.hpp"

PlaceableGeneralView::PlaceableGeneralView(nw::Placeable* obj, QWidget* parent)
    : QWidget(parent)
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
    ui->inventory->setEnabled(obj->has_inventory);
    ui->properties->setObject(obj_);

    auto& plcs = nw::kernel::rules().placeables.entries;
    uint32_t added = 0;
    std::string name;

    for (size_t i = 0; i < plcs.size(); ++i) {
        if (!plcs[i].valid()) { continue; }
        name = plcs[i].label;
        if (plcs[i].name != 0xFFFFFFFF) {
            name = nw::kernel::strings().get(plcs[i].name);
        }
        ui->appearance->addItem(to_qstring(name), int(i));
        if (i == obj_->appearance) {
            ui->appearance->setCurrentIndex(int(added));
        }
        ++added;
    }
    ui->appearance->model()->sort(0);

    connect(ui->appearance, &QComboBox::currentIndexChanged, this, &PlaceableGeneralView::onAppearanceChanged);
    connect(ui->variables, &QPushButton::clicked, this, &PlaceableGeneralView::onVariablesClicked);
}

PlaceableGeneralView::~PlaceableGeneralView()
{
    delete ui;
}

void PlaceableGeneralView::onAppearanceChanged(int value)
{
    obj_->appearance = static_cast<uint32_t>(ui->appearance->itemData(value).toInt());
    emit appearanceChanged();
}

void PlaceableGeneralView::onVariablesClicked()
{
    VarTableDialog dialog(this);
    dialog.setLocals(&obj_->common.locals);
    dialog.exec(); // This makes the dialog modal
}
