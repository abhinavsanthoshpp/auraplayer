/*
 * Orion Player — High-Performance Media Player for Linux
 * Copyright (C) 2026 Abhinav Santhosh (GitHub: @abhinavsanthoshpp)
 * All Rights Reserved.
 *
 * This software and its associated documentation, website, and design assets
 * are the intellectual property of Abhinav Santhosh. Unauthorized copying,
 * rebranding, redistribution, or commercial use is strictly prohibited.
 * See LICENSE file for full terms and conditions.
 */

#include "OrionVlcToolbar.h"
#include <QToolTip>
#include <QFrame>
#include <cmath>

/*
 * OrionVlcSlider implementation
 */
OrionVlcSlider::OrionVlcSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent) {
    setMouseTracking(true);
    setRange(0, 1000);
    setCursor(Qt::PointingHandCursor);
    setObjectName("OrionVlcTimeSlider");
}

double OrionVlcSlider::ratioFromX(int x) const {
    if (width() <= 0) return 0.0;
    return std::clamp(static_cast<double>(x) / static_cast<double>(width()), 0.0, 1.0);
}

void OrionVlcSlider::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        double r = ratioFromX(event->pos().x());
        setValue(static_cast<int>(r * 1000.0));
        emit seekPercent(r);
    }
    QSlider::mousePressEvent(event);
}

void OrionVlcSlider::mouseMoveEvent(QMouseEvent *event) {
    double r = ratioFromX(event->pos().x());
    emit hoverPercent(r, event->globalPosition().toPoint());
    QSlider::mouseMoveEvent(event);
}

void OrionVlcSlider::leaveEvent(QEvent *event) {
    QToolTip::hideText();
    QSlider::leaveEvent(event);
}

/*
 * OrionVlcToolbar implementation
 */
OrionVlcToolbar::OrionVlcToolbar(OrionEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine) {
    setupUi();

    if (m_engine) {
        connect(m_engine, &OrionEngine::positionChanged, this, &OrionVlcToolbar::setPosition);
        connect(m_engine, &OrionEngine::durationChanged, this, &OrionVlcToolbar::setDuration);
        connect(m_engine, &OrionEngine::playbackPaused, this, &OrionVlcToolbar::setPaused);
        connect(m_engine, &OrionEngine::volumeChanged, this, &OrionVlcToolbar::setVolume);
        connect(m_engine, &OrionEngine::muteChanged, this, &OrionVlcToolbar::setMuted);
    }
}

void OrionVlcToolbar::setupUi() {
    setObjectName("OrionVlcToolbar");
    setAttribute(Qt::WA_StyledBackground, true);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(8, 4, 8, 6);
    rootLayout->setSpacing(4);

    // 1. Advanced Controls Bar (Hidden by default, toggleable via View menu)
    m_advancedBar = new QWidget(this);
    m_advancedBar->setObjectName("OrionVlcAdvancedBar");
    auto *advLayout = new QHBoxLayout(m_advancedBar);
    advLayout->setContentsMargins(0, 0, 0, 0);
    advLayout->setSpacing(6);

    m_recordBtn = new QPushButton("🔴 Record", m_advancedBar);
    m_recordBtn->setToolTip("Record current playback stream");
    m_recordBtn->setFixedHeight(26);
    m_recordBtn->setFocusPolicy(Qt::NoFocus);

    m_snapshotBtn = new QPushButton("📷 Snapshot", m_advancedBar);
    m_snapshotBtn->setToolTip("Take video frame snapshot (Shift+S)");
    m_snapshotBtn->setFixedHeight(26);
    m_snapshotBtn->setFocusPolicy(Qt::NoFocus);

    m_abLoopBtn = new QPushButton("🔁 Loop A-B", m_advancedBar);
    m_abLoopBtn->setToolTip("Loop continuously between point A and point B");
    m_abLoopBtn->setFixedHeight(26);
    m_abLoopBtn->setFocusPolicy(Qt::NoFocus);

    m_frameStepBtn = new QPushButton("⏭ Frame", m_advancedBar);
    m_frameStepBtn->setToolTip("Step forward frame-by-frame (E)");
    m_frameStepBtn->setFixedHeight(26);
    m_frameStepBtn->setFocusPolicy(Qt::NoFocus);

    advLayout->addWidget(m_recordBtn);
    advLayout->addWidget(m_snapshotBtn);
    advLayout->addWidget(m_abLoopBtn);
    advLayout->addWidget(m_frameStepBtn);
    advLayout->addStretch(1);

    m_advancedBar->hide(); // default hidden like VLC
    rootLayout->addWidget(m_advancedBar);

    // 2. Time Progress Row (VLC-style: Elapsed Label on left, Slider in center, Duration on right)
    auto *timeRow = new QHBoxLayout();
    timeRow->setContentsMargins(0, 0, 0, 0);
    timeRow->setSpacing(8);

    m_elapsedLabel = new QLabel("00:00:00", this);
    m_elapsedLabel->setObjectName("OrionVlcElapsedLabel");

    m_timeSlider = new OrionVlcSlider(Qt::Horizontal, this);
    m_timeSlider->setFocusPolicy(Qt::NoFocus);

    m_durationLabel = new QLabel("00:00:00", this);
    m_durationLabel->setObjectName("OrionVlcDurationLabel");
    m_durationLabel->setCursor(Qt::PointingHandCursor);
    m_durationLabel->setToolTip("Click to toggle remaining time countdown");
    m_durationLabel->setFocusPolicy(Qt::NoFocus);

    timeRow->addWidget(m_elapsedLabel);
    timeRow->addWidget(m_timeSlider, 1);
    timeRow->addWidget(m_durationLabel);
    rootLayout->addLayout(timeRow);

    // 3. Main Transport & Controls Row
    auto *controlRow = new QHBoxLayout();
    controlRow->setContentsMargins(0, 0, 0, 0);
    controlRow->setSpacing(4);

    m_playPauseBtn = new QPushButton("▶", this);
    m_playPauseBtn->setObjectName("OrionVlcPlayPauseBtn");
    m_playPauseBtn->setFixedSize(32, 30);
    m_playPauseBtn->setToolTip("Play/Pause (Space)");
    m_playPauseBtn->setFocusPolicy(Qt::NoFocus);

    m_prevBtn = new QPushButton("⏮", this);
    m_prevBtn->setObjectName("OrionVlcToolBtn");
    m_prevBtn->setFixedSize(28, 28);
    m_prevBtn->setToolTip("Previous track in playlist (P)");
    m_prevBtn->setFocusPolicy(Qt::NoFocus);

    m_stopBtn = new QPushButton("⏹", this);
    m_stopBtn->setObjectName("OrionVlcToolBtn");
    m_stopBtn->setFixedSize(28, 28);
    m_stopBtn->setToolTip("Stop playback (S)");
    m_stopBtn->setFocusPolicy(Qt::NoFocus);

    m_nextBtn = new QPushButton("⏭", this);
    m_nextBtn->setObjectName("OrionVlcToolBtn");
    m_nextBtn->setFixedSize(28, 28);
    m_nextBtn->setToolTip("Next track in playlist (N)");
    m_nextBtn->setFocusPolicy(Qt::NoFocus);

    m_fullscreenBtn = new QPushButton("⛶", this);
    m_fullscreenBtn->setObjectName("OrionVlcToolBtn");
    m_fullscreenBtn->setFixedSize(28, 28);
    m_fullscreenBtn->setToolTip("Toggle Fullscreen (F11 / F)");
    m_fullscreenBtn->setFocusPolicy(Qt::NoFocus);

    m_effectsBtn = new QPushButton("🎛", this);
    m_effectsBtn->setObjectName("OrionVlcToolBtn");
    m_effectsBtn->setFixedSize(28, 28);
    m_effectsBtn->setToolTip("Show Extended Settings: Equalizer, Video FX, and Audio/Sub Sync (Ctrl+E)");
    m_effectsBtn->setFocusPolicy(Qt::NoFocus);

    m_playlistBtn = new QPushButton("📑", this);
    m_playlistBtn->setObjectName("OrionVlcToolBtn");
    m_playlistBtn->setFixedSize(28, 28);
    m_playlistBtn->setToolTip("Toggle Playlist View (Ctrl+L)");
    m_playlistBtn->setFocusPolicy(Qt::NoFocus);

    m_loopBtn = new QPushButton("➡️", this);
    m_loopBtn->setObjectName("OrionVlcToolBtn");
    m_loopBtn->setFixedSize(28, 28);
    m_loopBtn->setToolTip("Loop Mode: Normal (Click to toggle Repeat All / Repeat One)");
    m_loopBtn->setFocusPolicy(Qt::NoFocus);

    m_shuffleBtn = new QPushButton("🔀", this);
    m_shuffleBtn->setObjectName("OrionVlcToolBtn");
    m_shuffleBtn->setFixedSize(28, 28);
    m_shuffleBtn->setToolTip("Random / Shuffle Playlist Mode");
    m_shuffleBtn->setFocusPolicy(Qt::NoFocus);

    controlRow->addWidget(m_playPauseBtn);
    controlRow->addWidget(m_prevBtn);
    controlRow->addWidget(m_stopBtn);
    controlRow->addWidget(m_nextBtn);
    controlRow->addWidget(m_fullscreenBtn);
    controlRow->addWidget(m_effectsBtn);
    controlRow->addWidget(m_playlistBtn);
    controlRow->addWidget(m_loopBtn);
    controlRow->addWidget(m_shuffleBtn);

    controlRow->addStretch(1);

    // Audio Volume Section
    m_muteBtn = new QPushButton("🔊", this);
    m_muteBtn->setObjectName("OrionVlcMuteBtn");
    m_muteBtn->setFixedSize(26, 26);
    m_muteBtn->setToolTip("Mute / Unmute audio (M)");
    m_muteBtn->setFocusPolicy(Qt::NoFocus);

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setObjectName("OrionVlcVolSlider");
    m_volumeSlider->setRange(0, 200);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setFixedWidth(90);
    m_volumeSlider->setToolTip("Volume (0% - 200% with VLC audio booster)");
    m_volumeSlider->setFocusPolicy(Qt::NoFocus);

    m_volumeLabel = new QLabel("100%", this);
    m_volumeLabel->setObjectName("OrionVlcVolLabel");
    m_volumeLabel->setFixedWidth(42);
    m_volumeLabel->setFocusPolicy(Qt::NoFocus);

    controlRow->addWidget(m_muteBtn);
    controlRow->addWidget(m_volumeSlider);
    controlRow->addWidget(m_volumeLabel);

    rootLayout->addLayout(controlRow);

    // Signal connections
    connect(m_timeSlider, &OrionVlcSlider::seekPercent, this, &OrionVlcToolbar::onSeekRequested);
    connect(m_timeSlider, &OrionVlcSlider::hoverPercent, this, [this](double pct, const QPoint &pos) {
        if (m_duration > 0.0) {
            double hoverSecs = pct * m_duration;
            QToolTip::showText(pos, formatVlcTime(hoverSecs), m_timeSlider);
        }
    });

    connect(m_playPauseBtn, &QPushButton::clicked, this, &OrionVlcToolbar::playPauseClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &OrionVlcToolbar::stopClicked);
    connect(m_prevBtn, &QPushButton::clicked, this, &OrionVlcToolbar::prevClicked);
    connect(m_nextBtn, &QPushButton::clicked, this, &OrionVlcToolbar::nextClicked);
    connect(m_fullscreenBtn, &QPushButton::clicked, this, &OrionVlcToolbar::fullscreenClicked);
    connect(m_effectsBtn, &QPushButton::clicked, this, &OrionVlcToolbar::extendedSettingsClicked);
    connect(m_playlistBtn, &QPushButton::clicked, this, &OrionVlcToolbar::playlistClicked);
    connect(m_loopBtn, &QPushButton::clicked, this, &OrionVlcToolbar::onLoopClicked);
    connect(m_shuffleBtn, &QPushButton::clicked, this, &OrionVlcToolbar::shuffleClicked);

    connect(m_muteBtn, &QPushButton::clicked, this, &OrionVlcToolbar::onMuteClicked);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &OrionVlcToolbar::onVolumeChanged);

    connect(m_recordBtn, &QPushButton::clicked, this, &OrionVlcToolbar::recordClicked);
    connect(m_snapshotBtn, &QPushButton::clicked, this, &OrionVlcToolbar::snapshotClicked);
    connect(m_frameStepBtn, &QPushButton::clicked, this, &OrionVlcToolbar::frameStepClicked);

    m_durationLabel->installEventFilter(this);
}

void OrionVlcToolbar::setPaused(bool paused) {
    m_playPauseBtn->setText(paused ? "▶" : "⏸");
}

void OrionVlcToolbar::setPosition(double seconds) {
    m_position = seconds;
    m_elapsedLabel->setText(formatVlcTime(m_position));

    if (!m_timeSlider->isSliderDown() && m_duration > 0.0) {
        int val = static_cast<int>((m_position / m_duration) * 1000.0);
        m_timeSlider->setValue(std::clamp(val, 0, 1000));
    }

    if (m_showRemaining && m_duration > 0.0) {
        double rem = std::max(0.0, m_duration - m_position);
        m_durationLabel->setText(QString("-%1").arg(formatVlcTime(rem)));
    } else {
        m_durationLabel->setText(formatVlcTime(m_duration));
    }
}

void OrionVlcToolbar::setDuration(double seconds) {
    m_duration = seconds;
    setPosition(m_position);
}

void OrionVlcToolbar::setVolume(double volume) {
    m_volumeSlider->blockSignals(true);
    m_volumeSlider->setValue(static_cast<int>(volume));
    m_volumeSlider->blockSignals(false);
    m_volumeLabel->setText(QString("%1%").arg(static_cast<int>(volume)));

    if (volume <= 0.0) {
        m_muteBtn->setText("🔇");
    } else if (volume > 100.0) {
        m_muteBtn->setText("📢");
    } else {
        m_muteBtn->setText("🔊");
    }
}

void OrionVlcToolbar::setMuted(bool muted) {
    m_muteBtn->setText(muted ? "🔇" : "🔊");
}

void OrionVlcToolbar::setSpeed(double speed) {
    Q_UNUSED(speed);
}

void OrionVlcToolbar::setAdvancedControlsVisible(bool visible) {
    m_advancedBar->setVisible(visible);
}

bool OrionVlcToolbar::isAdvancedControlsVisible() const {
    return m_advancedBar->isVisible();
}

bool OrionVlcToolbar::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_durationLabel && event->type() == QEvent::MouseButtonPress) {
        onDurationLabelClicked();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void OrionVlcToolbar::onSeekRequested(double percent) {
    if (m_engine && m_duration > 0.0) {
        m_engine->seek(percent * m_duration);
    }
}

void OrionVlcToolbar::onVolumeChanged(int val) {
    m_volumeLabel->setText(QString("%1%").arg(val));
    if (m_engine) {
        m_engine->setVolume(static_cast<double>(val));
    }
}

void OrionVlcToolbar::onMuteClicked() {
    if (m_engine) {
        m_engine->toggleMute();
    }
}

void OrionVlcToolbar::onLoopClicked() {
    if (m_loopMode == LoopMode::None) {
        m_loopMode = LoopMode::RepeatAll;
        m_loopBtn->setText("🔁");
        m_loopBtn->setToolTip("Loop Mode: Repeat All");
    } else if (m_loopMode == LoopMode::RepeatAll) {
        m_loopMode = LoopMode::RepeatOne;
        m_loopBtn->setText("🔂");
        m_loopBtn->setToolTip("Loop Mode: Repeat One Track");
    } else {
        m_loopMode = LoopMode::None;
        m_loopBtn->setText("➡️");
        m_loopBtn->setToolTip("Loop Mode: Normal (No Repeat)");
    }
    emit loopModeChanged(m_loopMode);
}

void OrionVlcToolbar::onDurationLabelClicked() {
    m_showRemaining = !m_showRemaining;
    setPosition(m_position);
}

QString OrionVlcToolbar::formatVlcTime(double seconds) {
    if (seconds < 0.0) seconds = 0.0;
    int total = static_cast<int>(seconds);
    int h = total / 3600;
    int m = (total % 3600) / 60;
    int s = total % 60;

    return QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}
