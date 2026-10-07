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

class AuraSeekBar : public QSlider {
    Q_OBJECT

public:
    explicit AuraSeekBar(Qt::Orientation orientation, QWidget *parent = nullptr);

signals:
    void seekRequested(double positionPercent);
    void hoverPositionChanged(double percent, const QPoint &globalPos);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    double valueFromPosition(int x) const;
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
    void playlistToggleClicked();
    void equalizerClicked();
    void pipToggleClicked();
    void userInteracted();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

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

    AuraSeekBar *m_seekBar = nullptr;
    QLabel *m_timeLabel = nullptr;
    QPushButton *m_playPauseBtn = nullptr;
    QPushButton *m_stopBtn = nullptr;
    QPushButton *m_prevBtn = nullptr;
    QPushButton *m_nextBtn = nullptr;
    QPushButton *m_stepBackBtn = nullptr;
    QPushButton *m_stepFwdBtn = nullptr;

    QPushButton *m_muteBtn = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QLabel *m_volumeLabel = nullptr;

    QComboBox *m_speedCombo = nullptr;
    QComboBox *m_audioCombo = nullptr;
    QComboBox *m_subCombo = nullptr;
    QComboBox *m_aspectCombo = nullptr;

    QPushButton *m_eqBtn = nullptr;
    QPushButton *m_playlistBtn = nullptr;
    QPushButton *m_pipBtn = nullptr;
    QPushButton *m_fullscreenBtn = nullptr;

    double m_duration = 0.0;
    double m_currentPosition = 0.0;
    bool m_showRemaining = false;
    bool m_isSeeking = false;
};
