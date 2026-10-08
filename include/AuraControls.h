#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include "AuraEngine.h"

class AuraTimeline : public QSlider {
    Q_OBJECT

public:
    explicit AuraTimeline(Qt::Orientation orientation, QWidget *parent = nullptr);

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

class AuraControls : public QWidget {
    Q_OBJECT

public:
    explicit AuraControls(AuraEngine *engine, QWidget *parent = nullptr);

    void setPaused(bool paused);
    void setPosition(double seconds);
    void setDuration(double seconds);
    void setVolume(double volume);
    void setMuted(bool muted);
    void setSpeed(double speed);
    void updateTrackMenus();

signals:
    void playPauseClicked();
    void stopClicked();
    void nextClicked();
    void prevClicked();
    void stepForwardClicked();
    void stepBackClicked();
    void fullscreenClicked();
    void studioToggleClicked();
    void pipToggleClicked();
    void userInteracted();

private slots:
    void onSeekRequested(double percent);
    void onVolumeSliderChanged(int val);
    void onMuteBtnClicked();
    void onSpeedSelected(int index);
    void onAudioTrackSelected(int index);
    void onSubtitleTrackSelected(int index);
    void onAspectRatioSelected(int index);
    void onTimeLabelClicked();

private:
    void setupUi();
    static QString formatTime(double seconds);

    AuraEngine *m_engine = nullptr;

    AuraTimeline *m_timeline = nullptr;
    QLabel *m_timeLabel = nullptr;
    QPushButton *m_playPauseBtn = nullptr; // Central Aura Core
    QPushButton *m_prevBtn = nullptr;
    QPushButton *m_nextBtn = nullptr;
    QPushButton *m_stepBackBtn = nullptr;
    QPushButton *m_stepFwdBtn = nullptr;

    QPushButton *m_muteBtn = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QLabel *m_volumeBadge = nullptr;

    QComboBox *m_speedPill = nullptr;
    QComboBox *m_audioPill = nullptr;
    QComboBox *m_subPill = nullptr;
    QComboBox *m_aspectPill = nullptr;

    QPushButton *m_studioBtn = nullptr;
    QPushButton *m_pipBtn = nullptr;
    QPushButton *m_fullscreenBtn = nullptr;

    double m_duration = 0.0;
    double m_currentPosition = 0.0;
    bool m_showRemaining = false;
};
