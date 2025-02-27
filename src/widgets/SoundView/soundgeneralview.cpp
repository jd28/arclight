#include "soundgeneralview.h"
#include "ui_soundgeneralview.h"

#include "../../services/toolset/toolsetservice.h"
#include "../util/strings.h"

#include <nw/kernel/Resources.hpp>
#include <nw/objects/Sound.hpp>

#include <ZFontIcon/ZFontIcon.h>
#include <ZFontIcon/ZFont_fa6.h>

#include <QAudioOutput>
#include <QBuffer>
#include <QKeyEvent>
#include <QMediaPlayer>
#include <QStringListModel>

inline double scaleToDouble(int value)
{
    value = std::max(0, std::min(127, value));
    return (value / 127.0f) * 10.0f;
}

inline uint8_t scaleToInt(double value)
{
    value = std::max(0.0, std::min(10.0, value));
    return static_cast<uint8_t>(std::round((value / 10.0) * 127.0));
}

SoundGeneralView::SoundGeneralView(nw::Sound* obj, ArclightView* parent)
    : ArclightTab(parent)
    , ui(new Ui::SoundGeneralView)
    , obj_{obj}
{
    ui->setupUi(this);

    ui->name->setLocString(obj_->common.name);
    ui->tag->setText(to_qstring(obj_->tag().view()));
    ui->resref->setText(to_qstring(obj_->common.resref.view()));

    loadProperties();

    filter_ = new SoundSortFilterProxyModel(this);
    filter_->setSourceModel(toolset().sound_model.get());
    filter_->sort(0);
    ui->available->setModel(filter_);
#ifdef Q_OS_WINDOWS
    ui->available->setStyleSheet("QListView::item { height: 24px; }");
#endif
    connect(ui->available, &QListView::doubleClicked, this, &SoundGeneralView::onSoundDoubleClicked);

    auto model = new SoundModel(obj_->sounds, this);
    ui->current->setModel(model);
    ui->current->installEventFilter(this);
#ifdef Q_OS_WINDOWS
    ui->current->setStyleSheet("QListView::item { height: 24px; }");
#endif
    connect(ui->current, &QListView::doubleClicked, this, &SoundGeneralView::onSoundDoubleClicked);

    connect(ui->filter, &QLineEdit::textChanged, this, [this](const QString& text) {
        filter_->setFilter(text);
    });
}

SoundGeneralView::~SoundGeneralView()
{
    delete ui;
}

bool SoundGeneralView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->current && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            QModelIndex currentIndex = ui->current->currentIndex();
            if (currentIndex.isValid()) {
                auto model = static_cast<SoundModel*>(ui->current->model());
                model->removeSound(currentIndex.row());
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void SoundGeneralView::onSoundDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) { return; }
    auto text = index.data().toString();

    nw::ResourceData rdata;
    nw::Resref sound{text.toStdString()};
    for (auto rt : {nw::ResourceType::wav, nw::ResourceType::bmu}) {
        rdata = nw::kernel::resman().demand({sound, rt});
        if (rdata.bytes.size()) { break; }
    }

    if (rdata.bytes.size() == 0) {
        LOG_F(ERROR, "[arclight] failed to load audio file: {}", sound.view());
        return;
    }

    if (!player_) {
        player_ = new QMediaPlayer{this};
        output_ = new QAudioOutput(this);
        player_->setAudioOutput(output_);
    }

    const char* bytes = reinterpret_cast<const char*>(rdata.bytes.data());
    auto size = qsizetype(rdata.bytes.size());
    QUrl url;

    if (rdata.name.type == nw::ResourceType::bmu) {
        if (size < 8) {
            LOG_F(ERROR, "[arclight] invalid bmu file");
            return;
        }
        // Got to strip the BMU tag off, the rest is just mp3.
        if (std::strncmp(bytes, "BMU V1.0", 8) == 0) {
            bytes += 8;
        }
        url = to_qstring(rdata.name.resref.view()) + ".mp3";
    } else {
        url = to_qstring(rdata.name.filename());
    }

    QByteArray ba{bytes, size};
    output_->setVolume(50);

    QBuffer* buffer = new QBuffer(player_);
    buffer->setData(ba);
    if (!buffer->open(QIODevice::ReadOnly)) {
        LOG_F(ERROR, "audio buffer not opened");
    }

    buffer->seek(qint64(0));
    player_->setSourceDevice(buffer, url);
    player_->play();
}

void SoundGeneralView::loadProperties()
{
    ui->properties->setUndoStack(undoStack());

    auto p = ui->properties->makeBoolProperty("Active", obj_->active);
    p->on_set = [this](const QVariant& value) {
        obj_->active = value.toBool();
    };
    ui->properties->addProperty(p);

    auto grp = ui->properties->makeGroup("Playback");

    p = ui->properties->makeDoubleProperty("Interval", obj_->interval / 1000, grp);
    p->double_config.min = 0.0;
    p->double_config.max = 60.0;
    p->double_config.step = 0.1;
    p->on_set = [this](const QVariant& value) {
        obj_->interval = static_cast<float>(value.toDouble()) * 1000;
    };

    p = ui->properties->makeDoubleProperty("Interval Variation", obj_->interval_variation / 1000, grp);
    p->double_config.min = 0.0;
    p->double_config.max = 60.0;
    p->double_config.step = 0.1;
    p->on_set = [this](const QVariant& value) {
        obj_->interval_variation = static_cast<float>(value.toDouble()) * 1000;
    };

    QStringList order;
    order << "Sequential" << "Random";
    auto order_model = new QStringListModel(order, this);
    p = ui->properties->makeEnumProperty("Order", obj_->random, order_model, grp);
    p->on_set = [this](const QVariant& value) {
        obj_->random = value.toInt();
    };

    p = ui->properties->makeDoubleProperty("Pitch Variation", obj_->pitch_variation, grp);
    p->double_config.min = 0.0;
    p->double_config.max = 1.0;
    p->double_config.step = 0.01;
    p->on_set = [this](const QVariant& value) {
        obj_->pitch_variation = static_cast<float>(value.toDouble());
    };

    ui->properties->addProperty(grp);

    QStringList style;
    style << "Once" << "Repeating" << "Seemless Looping";
    auto style_model = new QStringListModel(style, this);
    int style_index = obj_->continuous ? 2 : obj_->looping ? 1
                                                           : 0;
    p = ui->properties->makeEnumProperty("Style", style_index, style_model, grp);
    p->on_set = [this](const QVariant& value) {
        switch (value.toInt()) {
        default:
            break;
        case 0:
            obj_->continuous = false;
            obj_->looping = false;
            break;
        case 1:
            obj_->continuous = false;
            obj_->looping = true;
            break;
        case 2:
            obj_->continuous = true;
            obj_->looping = false;
            break;
        }
    };

    QStringList times;
    times << "Specific Hours" << "Day" << "Night" << "Always";
    auto times_model = new QStringListModel(times, this);
    p = ui->properties->makeEnumProperty("Times", obj_->times, times_model, grp);
    p->on_set = [this](const QVariant& value) {
        obj_->times = static_cast<uint8_t>(value.toInt());
    };

    p = ui->properties->makeDoubleProperty("Volume", scaleToDouble(obj_->volume), grp);
    p->double_config.min = 0.0;
    p->double_config.max = 10.0;
    p->double_config.step = 0.01;
    p->on_set = [this](const QVariant& value) {
        obj_->volume = scaleToInt(value.toDouble());
    };

    p = ui->properties->makeDoubleProperty("Volume Variation", scaleToDouble(obj_->volume_variation), grp);
    p->double_config.min = 0.0;
    p->double_config.max = 10.0;
    p->double_config.step = 0.01;
    p->on_set = [this](const QVariant& value) {
        obj_->volume_variation = scaleToInt(value.toDouble());
    };

    grp = ui->properties->makeGroup("Positioning");

    QStringList pos;
    pos << "Everywhere" << "Random" << "Specific";
    auto model = new QStringListModel(pos, this);
    int index = obj_->random_position ? 1 : obj_->positional ? 2
                                                             : 0;
    p = ui->properties->makeEnumProperty("Location", index, model, grp);
    p->on_set = [this](const QVariant& value) {
        auto idx = value.toInt();
        switch (idx) {
        default:
            break;
        case 0:
            obj_->random_position = false;
            obj_->positional = false;
            break;
        case 1:
            obj_->random_position = true;
            obj_->positional = false;
            break;
        case 2:
            obj_->random_position = false;
            obj_->positional = true;
            break;
        }
    };

    p = ui->properties->makeDoubleProperty("Cutoff Distance (m)", obj_->distance_max, grp);
    p->double_config.min = 0.0;
    p->double_config.max = 1000.0;
    p->double_config.step = 0.1;
    p->on_set = [this](const QVariant& value) {
        obj_->distance_max = static_cast<float>(value.toDouble());
    };

    p = ui->properties->makeDoubleProperty("Full Volume Distance (m)", obj_->distance_min, grp);
    p->double_config.min = 0.0;
    p->double_config.max = 1000.0;
    p->double_config.step = 0.1;
    p->on_set = [this](const QVariant& value) {
        obj_->distance_min = static_cast<float>(value.toDouble());
    };

    p = ui->properties->makeDoubleProperty("Height (m)", obj_->elevation, grp);
    p->double_config.min = 0.0;
    p->double_config.max = 1000.0;
    p->double_config.step = 0.1;
    p->on_set = [this](const QVariant& value) {
        obj_->elevation = static_cast<float>(value.toDouble());
    };

    p = ui->properties->makeDoubleProperty("Random X Axis Range (m)", obj_->random_x, grp);
    p->double_config.min = 0.0;
    p->double_config.max = 640.0;
    p->double_config.step = 0.1;
    p->on_set = [this](const QVariant& value) {
        obj_->random_x = static_cast<float>(value.toDouble());
    };

    p = ui->properties->makeDoubleProperty("Random Y Axis Range (m)", obj_->random_y, grp);
    p->double_config.min = 0.0;
    p->double_config.max = 640.0;
    p->double_config.step = 0.1;
    p->on_set = [this](const QVariant& value) {
        obj_->random_y = static_cast<float>(value.toDouble());
    };

    ui->properties->addProperty(grp);
}
