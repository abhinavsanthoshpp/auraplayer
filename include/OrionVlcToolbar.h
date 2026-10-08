/*
 * Orion Player — High-Performance Open-Source Media Player for Linux
 * Copyright (C) 2026 Abhinav Santhosh <abhinavsanthoshpp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include "OrionEngine.h"

class OrionVlcSlider : public QSlider {
    Q_OBJECT

public:
    explicit OrionVlcSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

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

class OrionVlcToolbar : public QWidget {
    Q_OBJECT

public:
    enum class LoopMode {
        None,
        RepeatAll,
        RepeatOne
    };

    explicit OrionVlcToolbar(OrionEngine *engine, QWidget *parent = nullptr);

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

    OrionEngine *m_engine = nullptr;

    // Time Progress Row
    QLabel *m_elapsedLabel = nullptr;
    OrionVlcSlider *m_timeSlider = nullptr;
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
