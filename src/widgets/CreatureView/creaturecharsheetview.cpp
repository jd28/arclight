#include "creaturecharsheetview.h"
#include "ui_creaturecharsheetview.h"

#include "../../services/toolsetservice.h"
#include "../util/strings.h"
#include "creatureview.h"

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include "nw/kernel/Resources.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Creature.hpp"

CreatureCharSheetView::CreatureCharSheetView(nw::Creature* obj, CreatureView* parent)
    : ArclightTab(parent)
    , ui(new Ui::CreatureCharSheetView)
{
    ui->setupUi(this);
    loadCreature(obj);
    loadPortrait(obj);
    obj_ = obj;

    connect(this, &CreatureCharSheetView::modified, parent, &CreatureView::onModified);
}

CreatureCharSheetView::~CreatureCharSheetView()
{
    delete ui;
}

void CreatureCharSheetView::loadCreature(nw::Creature* obj)
{
    obj_ = nullptr;

    ui->firstName->setLocString(obj->name_first);
    ui->lastName->setLocString(obj->name_last);
    ui->tag->setText(to_qstring(obj->common.tag.view()));
    ui->resref->setText(to_qstring(obj->common.resref.view()));
    ui->genderSelector->setCurrentIndex(int(obj->gender));

    int first_invalid = -1;
    for (size_t i = 0; i < nw::LevelStats::max_classes; ++i) {
        auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(i + 1));
        auto combobox = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(i + 1));
        auto del = ui->classesWidget->findChild<QPushButton*>(QString("classDeleteButton_%1").arg(i + 1));

        connect(combobox, &QComboBox::currentIndexChanged, this, &CreatureCharSheetView::onClassChanged);
        connect(spinbox, &QSpinBox::valueChanged, this, &CreatureCharSheetView::onClassLevelChanged);
        connect(del, &QPushButton::clicked, this, &CreatureCharSheetView::onClassDeleteButtonClicked);

        auto proxy = toolset().class_filter.get();
        combobox->setModel(proxy);

        if (obj->levels.entries[i].id != nw::Class::invalid()) {
            QModelIndex sidx = toolset().class_model->index(*obj->levels.entries[i].id, 0, {});
            QModelIndex pidx = proxy->mapFromSource(sidx);
            combobox->setCurrentIndex(pidx.row());
            spinbox->setValue(obj->levels.entries[i].level);

            if (i == nw::LevelStats::max_classes - 1 || obj->levels.entries[i + 1].id == nw::Class::invalid()) {
                del->setEnabled(true);
            } else {
                del->setEnabled(false);
            }
        } else {
            if (first_invalid == -1) { first_invalid = int(i); }
            combobox->setCurrentIndex(-1);
            combobox->setDisabled(int(i) > first_invalid);
            spinbox->setDisabled(i > 0);
        }
    }

    QList<QPair<QString, int>> package_list;
    if (auto packages_2da = nw::kernel::twodas().get("packages")) {
        for (size_t i = 0; i < packages_2da->rows(); ++i) {
            if (auto package_class = packages_2da->get<int32_t>(i, "ClassID")) {
                if (obj->levels.level_by_class(nw::Class::make(*package_class)) <= 0) {
                    continue;
                }
            }
            if (auto name_id = packages_2da->get<int32_t>(i, "Name")) {
                auto name = nw::kernel::strings().get(uint32_t(*name_id));
                package_list.append({to_qstring(name), int(i)});
            }
        }
    }

    std::sort(package_list.begin(), package_list.end(), [](const QPair<QString, int>& lhs, const QPair<QString, int>& rhs) {
        return lhs.first < rhs.first;
    });

    int package_index = 0;
    for (int i = 0; i < package_list.size(); ++i) {
        ui->packageComboBox->addItem(package_list[i].first, package_list[i].second);
        if (obj->starting_package == static_cast<uint32_t>(package_list[i].second)) {
            package_index = i;
        }
    }
    ui->packageComboBox->setCurrentIndex(package_index);

    obj_ = obj;
}

void CreatureCharSheetView::loadPortrait(nw::Creature* obj)
{
    ui->portraitEdit->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_ellipsis));

    if (auto portraits_2da = nw::kernel::twodas().get("portraits")) {
        auto base = portraits_2da->get<std::string>(obj->appearance.portrait_id, "BaseResRef");
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
                if (img.is_bio_dds()) {
                    qi.mirror();
                }

                QRect rect(0, 0, 64, 100); // This is specific to medium portraits
                qi = qi.copy(rect);
                ui->labelPortraitImage->setPixmap(QPixmap::fromImage(qi));
                ui->portraitLineEdit->setText(to_qstring("po_" + *base));
            }
        }
    } else {
        LOG_F(ERROR, "Failed to load portraits.2da");
    }
}

// == Private Slots ===========================================================
// ============================================================================

void CreatureCharSheetView::onClassChanged(int index)
{
    if (!obj_ || index < 0) { return; }

    auto combobox = qobject_cast<QComboBox*>(sender());
    auto combobox_id = combobox->objectName().back().digitValue();

    auto class_slot = static_cast<size_t>(combobox_id - 1);
    auto class_id = nw::Class::make(combobox->currentData().toInt());
    bool new_class = obj_->levels.entries[class_slot].id == nw::Class::invalid();
    obj_->levels.entries[class_slot].id = class_id;

    auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(combobox_id));
    auto del = ui->classesWidget->findChild<QPushButton*>(QString("classDeleteButton_%1").arg(combobox_id));

    if (new_class) {
        if (class_slot > 0) {
            auto last_del = ui->classesWidget->findChild<QPushButton*>(QString("classDeleteButton_%1").arg(combobox_id - 1));
            last_del->setEnabled(false);
        }
        del->setEnabled(true);
        spinbox->setDisabled(false);
        spinbox->setValue(1);
        spinbox->setMinimum(1);

        if (combobox_id < 8) {
            auto combo = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(combobox_id + 1));
            combo->setEnabled(true);
        }

        emit classAdded(class_id);
    }
    emit modified();
}

void CreatureCharSheetView::onClassDeleteButtonClicked()
{
    auto widget = qobject_cast<QPushButton*>(sender());
    auto widget_id = widget->objectName().back().digitValue();
    widget->setEnabled(false);

    int class_slot = widget_id - 1;
    if (class_slot <= 0) { return; }
    auto last_class_id = obj_->levels.entries[class_slot].id;

    obj_->levels.entries[class_slot].id = nw::Class::invalid();
    obj_->levels.entries[class_slot].level = 0;

    auto combo = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(widget_id));
    combo->setCurrentIndex(-1);

    auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(widget_id));
    spinbox->setMinimum(0);
    spinbox->setValue(0);
    spinbox->setEnabled(false);

    if (widget_id > 2) { // Never have the first button enabled...
        auto del = ui->classesWidget->findChild<QPushButton*>(QString("classDeleteButton_%1").arg(widget_id - 1));
        del->setEnabled(true);
    }

    if (widget_id < 8) {
        auto next_combo = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(widget_id + 1));
        next_combo->setEnabled(false);
    }

    // [TODO] Clear all spells or let the spell editor handle that..
    // auto sb = obj_->levels.spells(last_class_id);

    emit classRemoved(last_class_id);
    emit modified();
}

void CreatureCharSheetView::onClassLevelChanged(int value)
{
    if (!obj_) { return; }

    auto spinbox = qobject_cast<QSpinBox*>(sender());
    auto class_slot = spinbox->objectName().back().digitValue() - 1;
    obj_->levels.entries[class_slot].level = int16_t(value);
    emit modified();
}
