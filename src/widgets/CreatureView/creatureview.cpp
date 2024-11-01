#include "creatureview.h"
#include "ui_creatureview.h"

#include "VariableTableView/vartabledialog.h"
#include "creatureappearanceview.h"
#include "creaturefeatselector.h"
#include "creaturestatsview.h"

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa6.h"

#include "nw/kernel/Resources.hpp"
#include "nw/kernel/Rules.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Creature.hpp"

#include <QApplication>
#include <QScreen>

CreatureView::CreatureView(nw::Creature* creature, QWidget* parent)
    : ArclightView(parent)
    , ui(new Ui::CreatureView)
{
    ui->setupUi(this);
    ui->openGLWidget->setCreature(creature);

    // Disable splitter movement for now, it's too easy to create weird results
    auto width = qApp->primaryScreen()->geometry().width();
    ui->splitter->setSizes(QList<int>() << int(width * 1.5 / 8) << int(width * 4.5 / 8) << int(width * 2 / 8));
    ui->splitter->handle(1)->setDisabled(true);
    ui->splitter->handle(2)->setDisabled(true);
    ui->tabWidget->addTab(new CreatureStatsView(creature, this), "Statistics");
    ui->tabWidget->addTab(new CreatureFeatSelector(creature, this), "Feats");
    auto appearance = new CreatureAppearanceView(creature, this);
    ui->tabWidget->addTab(appearance, "Appearance");
    connect(appearance, &CreatureAppearanceView::dataChanged, this, &CreatureView::onDataChanged);

    ui->portraitEdit->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_ellipsis));

    setupClassWidgets(creature);

    loadCreature(creature);
    connect(ui->variables, &QPushButton::clicked, this, &CreatureView::onVariablesClicked);
}

CreatureView::~CreatureView()
{
    delete ui;
}

void CreatureView::loadCreature(nw::Creature* creature)
{
    creature_ = nullptr;

    ui->firstName->setLocString(creature->name_first);
    ui->lastName->setLocString(creature->name_last);
    ui->genderSelector->setCurrentIndex(int(creature->gender));

    if (auto portraits_2da = nw::kernel::twodas().get("portraits")) {
        auto base = portraits_2da->get<std::string>(creature->appearance.portrait_id, "BaseResRef");
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
                ui->portraitLineEdit->setText(QString::fromStdString("po_" + *base));
            }
        }
    } else {
        LOG_F(ERROR, "Failed to load portraits.2da");
    }

    QList<QPair<QString, int>> race_list;
    for (size_t i = 0; i < nw::kernel::rules().races.entries.size(); ++i) {
        if (nw::kernel::rules().races.entries[i].name != 0xFFFFFFFF) {
            auto name = nw::kernel::strings().get(nw::kernel::rules().races.entries[i].name);
            race_list.append({QString::fromStdString(name), int(i)});
        }
    }

    std::sort(race_list.begin(), race_list.end(), [](const QPair<QString, int>& lhs, const QPair<QString, int>& rhs) {
        return lhs.first < rhs.first;
    });

    int race_index = 0;
    for (int i = 0; i < race_list.size(); ++i) {
        ui->race->addItem(race_list[i].first, race_list[i].second);
        if (creature->race == nw::Race::make(race_list[i].second)) {
            race_index = i;
        }
    }
    ui->race->setCurrentIndex(race_index);

    QList<QPair<QString, int>> class_list;
    for (size_t i = 0; i < nw::kernel::rules().classes.entries.size(); ++i) {
        if (nw::kernel::rules().classes.entries[i].name != 0xFFFFFFFF) {
            auto name = nw::kernel::strings().get(nw::kernel::rules().classes.entries[i].name);
            class_list.append({QString::fromStdString(name), int(i)});
        }
    }

    std::sort(class_list.begin(), class_list.end(), [](const QPair<QString, int>& lhs, const QPair<QString, int>& rhs) {
        return lhs.first < rhs.first;
    });

    for (size_t i = 0; i < nw::LevelStats::max_classes; ++i) {
        // auto widget = ui->classesWidget->findChild<QWidget*>(QString("classWidget_%1").arg(i + 1));
        auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(i + 1));
        auto combobox = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(i + 1));

        int class_index = -1;
        for (int j = 0; j < class_list.size(); ++j) {
            combobox->addItem(class_list[j].first, class_list[j].second);
            if (creature->levels.entries[i].id == nw::Class::make(class_list[j].second)) {
                class_index = j;
            }
        }

        if (creature->levels.entries[i].id != nw::Class::invalid()) {
            combobox->setCurrentIndex(class_index);
            spinbox->setValue(creature->levels.entries[i].level);
            ++current_classes_;
        } else if (i != 0) {
            spinbox->setDisabled(true);
        }
    }

    ui->classAdd->setEnabled(current_classes_ < 8);
    ui->classDelete->setEnabled(current_classes_ > 1);

    QList<QPair<QString, int>> package_list;
    if (auto packages_2da = nw::kernel::twodas().get("packages")) {
        for (size_t i = 0; i < packages_2da->rows(); ++i) {
            if (auto package_class = packages_2da->get<int32_t>(i, "ClassID")) {
                if (creature->levels.level_by_class(nw::Class::make(*package_class)) <= 0) {
                    continue;
                }
            }
            if (auto name_id = packages_2da->get<int32_t>(i, "Name")) {
                auto name = nw::kernel::strings().get(uint32_t(*name_id));
                package_list.append({QString::fromStdString(name), int(i)});
            }
        }
    }

    std::sort(package_list.begin(), package_list.end(), [](const QPair<QString, int>& lhs, const QPair<QString, int>& rhs) {
        return lhs.first < rhs.first;
    });

    int package_index = 0;
    for (int i = 0; i < package_list.size(); ++i) {
        ui->packageComboBox->addItem(package_list[i].first, package_list[i].second);
        if (creature->starting_package == package_list[i].second) {
            package_index = i;
        }
    }
    ui->packageComboBox->setCurrentIndex(package_index);

    creature_ = creature;
}

void CreatureView::setupClassWidgets(nw::Creature* creature)
{
    connect(ui->classAdd, &QPushButton::clicked, this, &CreatureView::onClassAddClicked);
    ui->classAdd->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_plus, Qt::green));
    connect(ui->classDelete, &QPushButton::clicked, this, &CreatureView::onClassDelClicked);
    ui->classDelete->setIcon(ZFontIcon::icon(Fa6::FAMILY, Fa6::fa_minus, Qt::red));
    ui->classDelete->setDisabled(creature->levels.entries[1].id == nw::Class::invalid());

    for (size_t i = 0; i < nw::LevelStats::max_classes; ++i) {
        auto widget = ui->classesWidget->findChild<QWidget*>(QString("classWidget_%1").arg(i + 1));
        auto combobox = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(i + 1));
        auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(i + 1));
        if (i > 0) {
            widget->setHidden(true);
        }
        combobox->setPlaceholderText(QStringLiteral("--Select Class--"));
        combobox->setCurrentIndex(-1);
        connect(combobox, &QComboBox::currentIndexChanged, this, &CreatureView::onClassChanged);
        connect(spinbox, &QSpinBox::valueChanged, this, &CreatureView::onClassLevelChanged);
    }
}

// == Public Slots ============================================================
// ============================================================================

void CreatureView::onClassAddClicked(bool checked)
{
    Q_UNUSED(checked);

    if (!creature_) { return; }

    // Don't add another class until the classes that are available are leveled
    if (current_classes_ > 0
        && creature_->levels.entries[size_t(current_classes_ - 1)].id == nw::Class::invalid()) {
        return;
    }

    size_t i;
    for (i = 0; i < nw::LevelStats::max_classes; ++i) {
        auto widget = ui->classesWidget->findChild<QWidget*>(QString("classWidget_%1").arg(i + 1));
        auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(i + 1));
        auto combobox = ui->classesWidget->findChild<QComboBox*>(QString("classComboBox_%1").arg(i + 1));

        if (!widget->isVisible()) {
            combobox->setPlaceholderText(QStringLiteral("--Select Class--"));
            combobox->setCurrentIndex(-1);
            spinbox->setValue(0);
            spinbox->setDisabled(true);
            widget->setVisible(true);
            ++current_classes_;
            break;
        }
    }

    ui->classAdd->setEnabled(current_classes_ < 8);
    ui->classDelete->setEnabled(current_classes_ > 1);
}

void CreatureView::onClassChanged(int index)
{
    if (!creature_) { return; }

    auto combobox = qobject_cast<QComboBox*>(sender());
    auto combobox_id = combobox->objectName().back().digitValue();

    auto class_slot = static_cast<size_t>(combobox_id - 1);
    auto class_id = nw::Class::make(combobox->currentData().toInt());
    creature_->levels.entries[class_slot].id = class_id;

    auto spinbox = ui->classesWidget->findChild<QSpinBox*>(QString("classLevelSpinBox_%1").arg(combobox_id));
    spinbox->setDisabled(false);
    spinbox->setValue(1);
    onDataChanged();
}

void CreatureView::onClassDelClicked(bool checked)
{
    Q_UNUSED(checked);

    if (!creature_) { return; }

    QWidget* widget = ui->classesWidget->findChild<QWidget*>(QString("classWidget_%1").arg(1));
    size_t i;

    for (i = 1; i < nw::LevelStats::max_classes; ++i) {
        auto next_widget = ui->classesWidget->findChild<QWidget*>(QString("classWidget_%1").arg(i + 1));
        if (!next_widget->isVisible()) { break; }
        widget = next_widget;
    }
    // Don't ever hide class one.
    if (i == 1) {
        ui->classDelete->setDisabled(true);
        return;
    }

    widget->setHidden(true);

    creature_->levels.entries[i - 1].id = nw::Class::invalid();
    creature_->levels.entries[i - 1].level = 0;
    creature_->levels.entries[i - 1].spells.known_.clear();
    creature_->levels.entries[i - 1].spells.memorized_.clear();

    --current_classes_;
    ui->classAdd->setEnabled(current_classes_ < 8);
    ui->classDelete->setEnabled(current_classes_ > 1);

    onDataChanged();
}

void CreatureView::onClassLevelChanged(int value)
{
    if (!creature_) { return; }

    auto spinbox = qobject_cast<QSpinBox*>(sender());
    auto class_slot = spinbox->objectName().back().digitValue() - 1;
    creature_->levels.entries[class_slot].level = int16_t(value);

    onDataChanged();
}

void CreatureView::onDataChanged()
{
    ui->openGLWidget->onDataChanged();
}

void CreatureView::onVariablesClicked()
{
    VarTableDialog dialog(this);
    dialog.setLocals(&creature_->common.locals);
    dialog.exec();
}
