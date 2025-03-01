#pragma once

#include "../arclighttab.h"

// == Forward Decls ===========================================================
// ============================================================================

namespace nw {
struct Sound;
}

namespace Ui {
class SoundGeneralView;
}

class SoundSortFilterProxyModel;
class QMediaPlayer;
class QAudioOutput;

// == SoundGeneralView ========================================================
// ============================================================================

class SoundGeneralView : public ArclightTab {
    Q_OBJECT

public:
    explicit SoundGeneralView(nw::Sound* obj, ArclightView* parent = nullptr);
    ~SoundGeneralView();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSoundDoubleClicked(const QModelIndex& index);

private:
    void loadProperties();

    Ui::SoundGeneralView* ui;
    nw::Sound* obj_;
    SoundSortFilterProxyModel* filter_;
    QMediaPlayer* player_ = nullptr;
    QAudioOutput* output_ = nullptr;
};
