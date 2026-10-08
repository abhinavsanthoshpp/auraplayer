#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include "AuraEngine.h"

class AuraVlcSlider : public QSlider {
    Q_OBJECT

public:
    explicit AuraVlcSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

signals:
    void seekPercent(double percent);
    void hoverPercent(double percent, const QPoint &globalPos);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    double ratioFromX(int x) const;
};

class AuraVlcToolbar : public QWidget {
    Q_OBJECT

public:
    enum class LoopMode {
        None,
        RepeatAll,
        RepeatOne
    };

    explicit AuraVlcToolbar(AuraEngine *engine, QWidget *parent = nullptr);

    void setPaused(bool paused);
    void setPosition(double seconds);
    void setDuration(double seconds);
    void setVolume(double volume);
    void setMuted(bool muted);
    void setSpeed(double speed);
    void setAdvancedControlsVisible(bool visible);
    bool isAdvancedControlsVisible() const;

signals:
    void playPauseClicked();
    void stopClicked();
    void prevClicked();
    void nextClicked();
    void fullscreenClicked();
    void extendedSettingsClicked();
    void playlistClicked();
    void snapshotClicked();
    void recordClicked();
    void frameStepClicked();
    void loopModeChanged(LoopMode mode);
    void shuffleClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onSeekRequested(double percent);
    void onVolumeChanged(int val);
    void onMuteClicked();
    void onLoopClicked();
    void onDurationLabelClicked();

private:
    void setupUi();
    static QString formatVlcTime(double seconds);

    AuraEngine *m_engine = nullptr;

    // Time Progress Row
    QLabel *m_elapsedLabel = nullptr;
    AuraVlcSlider *m_timeSlider = nullptr;
    QLabel *m_durationLabel = nullptr;

    // Main Control Row
    QPushButton *m_playPauseBtn = nullptr;
    QPushButton *m_prevBtn = nullptr;
    QPushButton *m_stopBtn = nullptr;
    QPushButton *m_nextBtn = nullptr;
    QPushButton *m_fullscreenBtn = nullptr;
    QPushButton *m_effectsBtn = nullptr;
    QPushButton *m_playlistBtn = nullptr;
    QPushButton *m_loopBtn = nullptr;
    QPushButton *m_shuffleBtn = nullptr;

    QPushButton *m_muteBtn = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QLabel *m_volumeLabel = nullptr;

    // Advanced Controls Row (Record, Snapshot, A-B, Frame Step)
    QWidget *m_advancedBar = nullptr;
    QPushButton *m_recordBtn = nullptr;
    QPushButton *m_snapshotBtn = nullptr;
    QPushButton *m_abLoopBtn = nullptr;
    QPushButton *m_frameStepBtn = nullptr;

    double m_duration = 0.0;
    double m_position = 0.0;
    bool m_showRemaining = false;
    LoopMode m_loopMode = LoopMode::None;
};
