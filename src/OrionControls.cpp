#include "OrionControls.h"
#include <QToolTip>
#include <cmath>

// ================= OrionTimeline =================

OrionTimeline::OrionTimeline(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent) {
    setMouseTracking(true);
    setRange(0, 1000);
    setCursor(Qt::PointingHandCursor);
    setObjectName("OrionHoloTimeline");
}

double OrionTimeline::ratioFromX(int x) const {
    if (width() <= 0) return 0.0;
    return std::clamp(static_cast<double>(x) / static_cast<double>(width()), 0.0, 1.0);
}

void OrionTimeline::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        double pct = ratioFromX(event->pos().x());
        setValue(static_cast<int>(pct * 1000.0));
        emit seekPercent(pct);
    }
    QSlider::mousePressEvent(event);
}

void OrionTimeline::mouseMoveEvent(QMouseEvent *event) {
    double pct = ratioFromX(event->pos().x());
    emit hoverPercent(pct, event->globalPosition().toPoint());
    QSlider::mouseMoveEvent(event);
}

void OrionTimeline::leaveEvent(QEvent *event) {
    QToolTip::hideText();
    QSlider::leaveEvent(event);
}

// ================= OrionControls (Cyber Deck) =================

OrionControls::OrionControls(OrionEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine) {
    setupUi();

    if (m_engine) {
        connect(m_engine, &OrionEngine::positionChanged, this, &OrionControls::setPosition);
        connect(m_engine, &OrionEngine::durationChanged, this, &OrionControls::setDuration);
        connect(m_engine, &OrionEngine::playbackPaused, this, &OrionControls::setPaused);
        connect(m_engine, &OrionEngine::volumeChanged, this, &OrionControls::setVolume);
        connect(m_engine, &OrionEngine::muteChanged, this, &OrionControls::setMuted);
        connect(m_engine, &OrionEngine::speedChanged, this, &OrionControls::setSpeed);
        connect(m_engine, &OrionEngine::tracksChanged, this, &OrionControls::updateTrackMenus);
    }
}

void OrionControls::setupUi() {
    setObjectName("OrionCyberDeck");
    setAttribute(Qt::WA_StyledBackground, true);

    auto *dockLayout = new QVBoxLayout(this);
    dockLayout->setContentsMargins(18, 10, 18, 12);
    dockLayout->setSpacing(8);

    // 1. Holographic Timeline Row
    auto *timelineRow = new QHBoxLayout();
    timelineRow->setSpacing(12);

    m_timeline = new OrionTimeline(Qt::Horizontal, this);
    m_timeLabel = new QLabel("00:00 / 00:00", this);
    m_timeLabel->setObjectName("OrionTimeBadge");
    m_timeLabel->setCursor(Qt::PointingHandCursor);
    m_timeLabel->setToolTip("Click to toggle remaining time countdown");

    timelineRow->addWidget(m_timeline, 1);
    timelineRow->addWidget(m_timeLabel, 0);
    dockLayout->addLayout(timelineRow);

    // 2. Main Cyber Deck Controls Row
    auto *controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(10);

    // Left Wing: Sound Capsule
    auto *soundCapsule = new QWidget(this);
    soundCapsule->setObjectName("OrionSoundCapsule");
    auto *soundLayout = new QHBoxLayout(soundCapsule);
    soundLayout->setContentsMargins(8, 2, 10, 2);
    soundLayout->setSpacing(6);

    m_muteBtn = new QPushButton("🔊", soundCapsule);
    m_muteBtn->setObjectName("OrionMutePill");
    m_muteBtn->setFixedSize(28, 28);
    m_muteBtn->setToolTip("Mute / Unmute (M)");

    m_volumeSlider = new QSlider(Qt::Horizontal, soundCapsule);
    m_volumeSlider->setObjectName("OrionFluidVolSlider");
    m_volumeSlider->setRange(0, 200);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setFixedWidth(75);
    m_volumeSlider->setToolTip("Volume (0% - 200% Supercharged Boost)");

    m_volumeBadge = new QLabel("100%", soundCapsule);
    m_volumeBadge->setObjectName("OrionVolBadge");
    m_volumeBadge->setFixedWidth(36);

    soundLayout->addWidget(m_muteBtn);
    soundLayout->addWidget(m_volumeSlider);
    soundLayout->addWidget(m_volumeBadge);
    controlsRow->addWidget(soundCapsule);

    controlsRow->addStretch(1);

    // Center Core: Transport Navigation & Pulsing Orion Core Play Button
    auto *coreCluster = new QWidget(this);
    coreCluster->setObjectName("OrionCoreCluster");
    auto *coreLayout = new QHBoxLayout(coreCluster);
    coreLayout->setContentsMargins(6, 2, 6, 2);
    coreLayout->setSpacing(8);

    m_prevBtn = new QPushButton("⏮", coreCluster);
    m_prevBtn->setObjectName("OrionNavBtn");
    m_prevBtn->setToolTip("Previous Track (P)");
    m_prevBtn->setFixedSize(30, 30);

    m_stepBackBtn = new QPushButton("‹", coreCluster);
    m_stepBackBtn->setObjectName("OrionStepBtn");
    m_stepBackBtn->setToolTip("Step Frame Backward");
    m_stepBackBtn->setFixedSize(26, 26);

    m_playPauseBtn = new QPushButton("▶", coreCluster);
    m_playPauseBtn->setObjectName("OrionPlayCore");
    m_playPauseBtn->setToolTip("Play / Pause (Space)");
    m_playPauseBtn->setFixedSize(48, 48);

    m_stepFwdBtn = new QPushButton("›", coreCluster);
    m_stepFwdBtn->setObjectName("OrionStepBtn");
    m_stepFwdBtn->setToolTip("Step Frame Forward");
    m_stepFwdBtn->setFixedSize(26, 26);

    m_nextBtn = new QPushButton("⏭", coreCluster);
    m_nextBtn->setObjectName("OrionNavBtn");
    m_nextBtn->setToolTip("Next Track (N)");
    m_nextBtn->setFixedSize(30, 30);

    coreLayout->addWidget(m_prevBtn);
    coreLayout->addWidget(m_stepBackBtn);
    coreLayout->addWidget(m_playPauseBtn);
    coreLayout->addWidget(m_stepFwdBtn);
    coreLayout->addWidget(m_nextBtn);
    controlsRow->addWidget(coreCluster);

    controlsRow->addStretch(1);

    // Right Wing: Stream Pills & Studio Toggle
    m_speedPill = new QComboBox(this);
    m_speedPill->setObjectName("OrionPillCombo");
    m_speedPill->setToolTip("Playback Speed Multiplier");
    m_speedPill->addItem("0.5x", 0.5);
    m_speedPill->addItem("0.75x", 0.75);
    m_speedPill->addItem("1.0x", 1.0);
    m_speedPill->addItem("1.25x", 1.25);
    m_speedPill->addItem("1.5x", 1.5);
    m_speedPill->addItem("2.0x", 2.0);
    m_speedPill->addItem("3.0x", 3.0);
    m_speedPill->addItem("4.0x", 4.0);
    m_speedPill->setCurrentIndex(2);
    m_speedPill->setFixedWidth(68);

    m_audioPill = new QComboBox(this);
    m_audioPill->setObjectName("OrionPillCombo");
    m_audioPill->setToolTip("Audio Stream Track");
    m_audioPill->addItem("Audio", -1);
    m_audioPill->setMaximumWidth(88);

    m_subPill = new QComboBox(this);
    m_subPill->setObjectName("OrionPillCombo");
    m_subPill->setToolTip("Subtitle Track");
    m_subPill->addItem("Subs", -1);
    m_subPill->setMaximumWidth(88);

    m_studioBtn = new QPushButton("⚡ Studio", this);
    m_studioBtn->setObjectName("OrionStudioLaunchPill");
    m_studioBtn->setToolTip("Toggle Orion Studio Panel (L / Tab)");

    m_pipBtn = new QPushButton("📌", this);
    m_pipBtn->setObjectName("OrionUtilityPill");
    m_pipBtn->setToolTip("Always on Top (Picture-in-Picture)");
    m_pipBtn->setFixedSize(32, 32);

    m_fullscreenBtn = new QPushButton("⛶", this);
    m_fullscreenBtn->setObjectName("OrionUtilityPill");
    m_fullscreenBtn->setToolTip("Immersive Fullscreen (F / F11)");
    m_fullscreenBtn->setFixedSize(32, 32);

    controlsRow->addWidget(m_speedPill);
    controlsRow->addWidget(m_audioPill);
    controlsRow->addWidget(m_subPill);
    controlsRow->addWidget(m_studioBtn);
    controlsRow->addWidget(m_pipBtn);
    controlsRow->addWidget(m_fullscreenBtn);

    dockLayout->addLayout(controlsRow);

    // Timeline event connections
    connect(m_timeline, &OrionTimeline::seekPercent, this, &OrionControls::onSeekRequested);
    connect(m_timeline, &OrionTimeline::hoverPercent, this, [this](double pct, const QPoint &pos) {
        if (m_duration > 0.0) {
            double hoverSecs = pct * m_duration;
            QToolTip::showText(pos, formatTime(hoverSecs), m_timeline);
        }
    });

    // Button connections
    connect(m_playPauseBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit playPauseClicked();
    });
    connect(m_prevBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit prevClicked();
    });
    connect(m_nextBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit nextClicked();
    });
    connect(m_stepBackBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit stepBackClicked();
    });
    connect(m_stepFwdBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit stepForwardClicked();
    });

    connect(m_muteBtn, &QPushButton::clicked, this, &OrionControls::onMuteBtnClicked);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &OrionControls::onVolumeSliderChanged);

    connect(m_speedPill, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrionControls::onSpeedSelected);
    connect(m_audioPill, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrionControls::onAudioTrackSelected);
    connect(m_subPill, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrionControls::onSubtitleTrackSelected);

    connect(m_studioBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit studioToggleClicked();
    });
    connect(m_pipBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit pipToggleClicked();
    });
    connect(m_fullscreenBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit fullscreenClicked();
    });

    // Make time label interactive
    m_timeLabel->setMouseTracking(true);
}

void OrionControls::setPaused(bool paused) {
    m_playPauseBtn->setText(paused ? "▶" : "⏸");
}

void OrionControls::setPosition(double seconds) {
    m_currentPosition = seconds;
    if (!m_timeline->isSliderDown() && m_duration > 0.0) {
        int sliderVal = static_cast<int>((m_currentPosition / m_duration) * 1000.0);
        m_timeline->setValue(std::clamp(sliderVal, 0, 1000));
    }

    if (m_showRemaining && m_duration > 0.0) {
        double rem = std::max(0.0, m_duration - m_currentPosition);
        m_timeLabel->setText(QString("%1 / -%2").arg(formatTime(m_currentPosition), formatTime(rem)));
    } else {
        m_timeLabel->setText(QString("%1 / %2").arg(formatTime(m_currentPosition), formatTime(m_duration)));
    }
}

void OrionControls::setDuration(double seconds) {
    m_duration = seconds;
    setPosition(m_currentPosition);
}

void OrionControls::setVolume(double volume) {
    m_volumeSlider->blockSignals(true);
    m_volumeSlider->setValue(static_cast<int>(volume));
    m_volumeSlider->blockSignals(false);
    m_volumeBadge->setText(QString("%1%").arg(static_cast<int>(volume)));

    if (volume <= 0.0) {
        m_muteBtn->setText("🔇");
        m_volumeBadge->setStyleSheet("color: #6e7681;");
    } else if (volume > 100.0) {
        m_muteBtn->setText("⚡");
        m_volumeBadge->setStyleSheet("color: #ff9900; font-weight: bold;"); // Supercharged boost indicator
    } else {
        m_muteBtn->setText("🔊");
        m_volumeBadge->setStyleSheet("color: #00e5ff;");
    }
}

void OrionControls::setMuted(bool muted) {
    m_muteBtn->setText(muted ? "🔇" : "🔊");
}

void OrionControls::setSpeed(double speed) {
    for (int i = 0; i < m_speedPill->count(); ++i) {
        if (std::abs(m_speedPill->itemData(i).toDouble() - speed) < 0.05) {
            m_speedPill->blockSignals(true);
            m_speedPill->setCurrentIndex(i);
            m_speedPill->blockSignals(false);
            break;
        }
    }
}

void OrionControls::onSeekRequested(double percent) {
    emit userInteracted();
    if (m_engine && m_duration > 0.0) {
        m_engine->seek(percent * m_duration);
    }
}

void OrionControls::onVolumeSliderChanged(int val) {
    emit userInteracted();
    setVolume(static_cast<double>(val));
    if (m_engine) {
        m_engine->setVolume(static_cast<double>(val));
    }
}

void OrionControls::onMuteBtnClicked() {
    emit userInteracted();
    if (m_engine) {
        m_engine->toggleMute();
    }
}

void OrionControls::onSpeedSelected(int index) {
    emit userInteracted();
    if (m_engine && index >= 0) {
        m_engine->setSpeed(m_speedPill->itemData(index).toDouble());
    }
}

void OrionControls::updateTrackMenus() {
    if (!m_engine) return;
    auto tracks = m_engine->getTracks();

    m_audioPill->blockSignals(true);
    m_audioPill->clear();
    m_audioPill->addItem("Audio", -1);
    for (const auto &track : tracks) {
        if (track.type == "audio") {
            QString lang = track.language.isEmpty() ? QString("#%1").arg(track.id) : track.language.toUpper();
            m_audioPill->addItem(lang, track.id);
            if (track.isSelected) {
                m_audioPill->setCurrentIndex(m_audioPill->count() - 1);
            }
        }
    }
    m_audioPill->blockSignals(false);

    m_subPill->blockSignals(true);
    m_subPill->clear();
    m_subPill->addItem("Subs: Off", -1);
    for (const auto &track : tracks) {
        if (track.type == "sub") {
            QString lang = track.language.isEmpty() ? QString("#%1").arg(track.id) : track.language.toUpper();
            m_subPill->addItem(lang, track.id);
            if (track.isSelected) {
                m_subPill->setCurrentIndex(m_subPill->count() - 1);
            }
        }
    }
    m_subPill->blockSignals(false);
}

void OrionControls::onAudioTrackSelected(int index) {
    emit userInteracted();
    if (m_engine && index >= 0) {
        m_engine->setAudioTrack(m_audioPill->itemData(index).toInt());
    }
}

void OrionControls::onSubtitleTrackSelected(int index) {
    emit userInteracted();
    if (m_engine && index >= 0) {
        m_engine->setSubtitleTrack(m_subPill->itemData(index).toInt());
    }
}

void OrionControls::onAspectRatioSelected(int index) {
    Q_UNUSED(index);
}

void OrionControls::onTimeLabelClicked() {
    emit userInteracted();
    m_showRemaining = !m_showRemaining;
    setPosition(m_currentPosition);
}

QString OrionControls::formatTime(double seconds) {
    if (seconds < 0.0) seconds = 0.0;
    int total = static_cast<int>(seconds);
    int h = total / 3600;
    int m = (total % 3600) / 60;
    int s = total % 60;
    if (h > 0) {
        return QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    }
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}
