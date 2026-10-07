#include "AuraControls.h"
#include <QToolTip>
#include <cmath>

// ================= AuraSeekBar Implementation =================

AuraSeekBar::AuraSeekBar(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent) {
    setMouseTracking(true);
    setRange(0, 1000);
    setCursor(Qt::PointingHandCursor);
}

double AuraSeekBar::valueFromPosition(int x) const {
    if (width() <= 0) return 0.0;
    double ratio = static_cast<double>(x) / static_cast<double>(width());
    return std::clamp(ratio, 0.0, 1.0);
}

void AuraSeekBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        double pct = valueFromPosition(event->pos().x());
        setValue(static_cast<int>(pct * 1000.0));
        emit seekRequested(pct);
    }
    QSlider::mousePressEvent(event);
}

void AuraSeekBar::mouseMoveEvent(QMouseEvent *event) {
    double pct = valueFromPosition(event->pos().x());
    emit hoverPositionChanged(pct, event->globalPosition().toPoint());
    QSlider::mouseMoveEvent(event);
}

void AuraSeekBar::enterEvent(QEnterEvent *event) {
    QSlider::enterEvent(event);
}

void AuraSeekBar::leaveEvent(QEvent *event) {
    QToolTip::hideText();
    QSlider::leaveEvent(event);
}

// ================= AuraControls Implementation =================

AuraControls::AuraControls(AuraEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine) {
    setupUi();

    if (m_engine) {
        connect(m_engine, &AuraEngine::positionChanged, this, &AuraControls::setPosition);
        connect(m_engine, &AuraEngine::durationChanged, this, &AuraControls::setDuration);
        connect(m_engine, &AuraEngine::playbackPaused, this, &AuraControls::setPaused);
        connect(m_engine, &AuraEngine::volumeChanged, this, &AuraControls::setVolume);
        connect(m_engine, &AuraEngine::muteChanged, this, &AuraControls::setMuted);
        connect(m_engine, &AuraEngine::speedChanged, this, &AuraControls::setSpeed);
        connect(m_engine, &AuraEngine::tracksChanged, this, &AuraControls::updateTrackMenus);
    }
}

void AuraControls::setupUi() {
    setObjectName("AuraControlsPanel");
    setAttribute(Qt::WA_StyledBackground, true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 8, 14, 10);
    mainLayout->setSpacing(6);

    // 1. Seek bar & Time row
    auto *seekLayout = new QHBoxLayout();
    seekLayout->setSpacing(10);

    m_seekBar = new AuraSeekBar(Qt::Horizontal, this);
    m_seekBar->setObjectName("AuraTimelineSlider");

    m_timeLabel = new QLabel("00:00 / 00:00", this);
    m_timeLabel->setObjectName("AuraTimeLabel");
    m_timeLabel->setCursor(Qt::PointingHandCursor);
    m_timeLabel->setToolTip("Click to toggle remaining time");

    seekLayout->addWidget(m_seekBar, 1);
    seekLayout->addWidget(m_timeLabel, 0);
    mainLayout->addLayout(seekLayout);

    // 2. Playback buttons & controls row
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);

    // Media navigation
    m_prevBtn = new QPushButton("⏮", this);
    m_prevBtn->setToolTip("Previous Track");
    m_prevBtn->setFixedSize(34, 34);

    m_stepBackBtn = new QPushButton("⏪", this);
    m_stepBackBtn->setToolTip("Step Frame Backward");
    m_stepBackBtn->setFixedSize(30, 34);

    m_playPauseBtn = new QPushButton("▶", this);
    m_playPauseBtn->setObjectName("AuraPlayPauseButton");
    m_playPauseBtn->setToolTip("Play / Pause (Space)");
    m_playPauseBtn->setFixedSize(42, 38);

    m_stepFwdBtn = new QPushButton("⏩", this);
    m_stepFwdBtn->setToolTip("Step Frame Forward");
    m_stepFwdBtn->setFixedSize(30, 34);

    m_nextBtn = new QPushButton("⏭", this);
    m_nextBtn->setToolTip("Next Track");
    m_nextBtn->setFixedSize(34, 34);

    m_stopBtn = new QPushButton("⏹", this);
    m_stopBtn->setToolTip("Stop Playback");
    m_stopBtn->setFixedSize(34, 34);

    btnRow->addWidget(m_prevBtn);
    btnRow->addWidget(m_stepBackBtn);
    btnRow->addWidget(m_playPauseBtn);
    btnRow->addWidget(m_stepFwdBtn);
    btnRow->addWidget(m_nextBtn);
    btnRow->addWidget(m_stopBtn);

    // Volume group
    btnRow->addSpacing(10);
    m_muteBtn = new QPushButton("🔊", this);
    m_muteBtn->setToolTip("Mute / Unmute (M)");
    m_muteBtn->setFixedSize(34, 34);

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setObjectName("AuraVolumeSlider");
    m_volumeSlider->setRange(0, 200);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setFixedWidth(90);
    m_volumeSlider->setToolTip("Volume (0% - 200% Boost)");

    m_volumeLabel = new QLabel("100%", this);
    m_volumeLabel->setObjectName("AuraVolumeLabel");
    m_volumeLabel->setFixedWidth(40);

    btnRow->addWidget(m_muteBtn);
    btnRow->addWidget(m_volumeSlider);
    btnRow->addWidget(m_volumeLabel);

    btnRow->addStretch(1);

    // Speed selector
    m_speedCombo = new QComboBox(this);
    m_speedCombo->setToolTip("Playback Speed");
    m_speedCombo->addItem("0.50x", 0.5);
    m_speedCombo->addItem("0.75x", 0.75);
    m_speedCombo->addItem("1.00x", 1.0);
    m_speedCombo->addItem("1.25x", 1.25);
    m_speedCombo->addItem("1.50x", 1.5);
    m_speedCombo->addItem("2.00x", 2.0);
    m_speedCombo->addItem("3.00x", 3.0);
    m_speedCombo->addItem("4.00x", 4.0);
    m_speedCombo->setCurrentIndex(2);
    m_speedCombo->setFixedWidth(78);

    // Audio & Subtitle selectors
    m_audioCombo = new QComboBox(this);
    m_audioCombo->setToolTip("Audio Stream");
    m_audioCombo->addItem("Audio: Auto", -1);
    m_audioCombo->setMaximumWidth(110);

    m_subCombo = new QComboBox(this);
    m_subCombo->setToolTip("Subtitles Track");
    m_subCombo->addItem("Subs: None", -1);
    m_subCombo->setMaximumWidth(110);

    // Aspect ratio
    m_aspectCombo = new QComboBox(this);
    m_aspectCombo->setToolTip("Aspect Ratio");
    m_aspectCombo->addItem("Aspect: Auto", "default");
    m_aspectCombo->addItem("16:9", "16:9");
    m_aspectCombo->addItem("4:3", "4:3");
    m_aspectCombo->addItem("21:9", "21:9");
    m_aspectCombo->addItem("Fill", "fill");
    m_aspectCombo->setMaximumWidth(95);

    btnRow->addWidget(m_speedCombo);
    btnRow->addWidget(m_audioCombo);
    btnRow->addWidget(m_subCombo);
    btnRow->addWidget(m_aspectCombo);

    // Tools & views
    btnRow->addSpacing(6);
    m_eqBtn = new QPushButton("🎚️", this);
    m_eqBtn->setToolTip("Audio / Video Equalizer (E / C)");
    m_eqBtn->setFixedSize(34, 34);

    m_playlistBtn = new QPushButton("📑", this);
    m_playlistBtn->setToolTip("Toggle Playlist Drawer (L)");
    m_playlistBtn->setFixedSize(34, 34);

    m_pipBtn = new QPushButton("📌", this);
    m_pipBtn->setToolTip("Always on Top (Picture-in-Picture)");
    m_pipBtn->setFixedSize(34, 34);

    m_fullscreenBtn = new QPushButton("⛶", this);
    m_fullscreenBtn->setToolTip("Fullscreen Toggle (F / F11)");
    m_fullscreenBtn->setFixedSize(34, 34);

    btnRow->addWidget(m_eqBtn);
    btnRow->addWidget(m_playlistBtn);
    btnRow->addWidget(m_pipBtn);
    btnRow->addWidget(m_fullscreenBtn);

    mainLayout->addLayout(btnRow);

    // Signal connections
    connect(m_seekBar, &AuraSeekBar::seekRequested, this, &AuraControls::onSeekRequested);
    connect(m_seekBar, &AuraSeekBar::hoverPositionChanged, this, [this](double pct, const QPoint &globalPos) {
        if (m_duration > 0.0) {
            double hoverSecs = pct * m_duration;
            QToolTip::showText(globalPos, formatTime(hoverSecs), m_seekBar);
        }
    });

    connect(m_playPauseBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit playPauseClicked();
    });
    connect(m_stopBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit stopClicked();
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
    connect(m_muteBtn, &QPushButton::clicked, this, &AuraControls::onMuteBtnClicked);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &AuraControls::onVolumeSliderChanged);

    connect(m_speedCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &AuraControls::onSpeedSelected);
    connect(m_audioCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AuraControls::onAudioTrackSelected);
    connect(m_subCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AuraControls::onSubtitleTrackSelected);
    connect(m_aspectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AuraControls::onAspectRatioSelected);

    connect(m_eqBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit equalizerClicked();
    });
    connect(m_playlistBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit playlistToggleClicked();
    });
    connect(m_pipBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit pipToggleClicked();
    });
    connect(m_fullscreenBtn, &QPushButton::clicked, this, [this]() {
        emit userInteracted();
        emit fullscreenClicked();
    });

    // Make time label clickable to toggle remaining time
    // Install event filter for click
    m_timeLabel->installEventFilter(this);
}

void AuraControls::setPaused(bool paused) {
    m_playPauseBtn->setText(paused ? "▶" : "⏸");
}

void AuraControls::setPosition(double seconds) {
    m_currentPosition = seconds;
    if (!m_seekBar->isSliderDown() && m_duration > 0.0) {
        int sliderVal = static_cast<int>((m_currentPosition / m_duration) * 1000.0);
        m_seekBar->setValue(std::clamp(sliderVal, 0, 1000));
    }

    if (m_showRemaining && m_duration > 0.0) {
        double rem = std::max(0.0, m_duration - m_currentPosition);
        m_timeLabel->setText(QString("%1 / -%2").arg(formatTime(m_currentPosition), formatTime(rem)));
    } else {
        m_timeLabel->setText(QString("%1 / %2").arg(formatTime(m_currentPosition), formatTime(m_duration)));
    }
}

void AuraControls::setDuration(double seconds) {
    m_duration = seconds;
    setPosition(m_currentPosition);
}

void AuraControls::setVolume(double volume) {
    m_volumeSlider->blockSignals(true);
    m_volumeSlider->setValue(static_cast<int>(volume));
    m_volumeSlider->blockSignals(false);
    m_volumeLabel->setText(QString("%1%").arg(static_cast<int>(volume)));
    if (volume <= 0.0) {
        m_muteBtn->setText("🔇");
    } else if (volume > 100.0) {
        m_muteBtn->setText("📢"); // Volume boosted
    } else {
        m_muteBtn->setText("🔊");
    }
}

void AuraControls::setMuted(bool muted) {
    m_muteBtn->setText(muted ? "🔇" : "🔊");
}

void AuraControls::setSpeed(double speed) {
    for (int i = 0; i < m_speedCombo->count(); ++i) {
        if (std::abs(m_speedCombo->itemData(i).toDouble() - speed) < 0.05) {
            m_speedCombo->blockSignals(true);
            m_speedCombo->setCurrentIndex(i);
            m_speedCombo->blockSignals(false);
            break;
        }
    }
}

void AuraControls::onSeekRequested(double percent) {
    emit userInteracted();
    if (m_engine && m_duration > 0.0) {
        double targetSecs = percent * m_duration;
        m_engine->seek(targetSecs);
    }
}

void AuraControls::onVolumeSliderChanged(int val) {
    emit userInteracted();
    m_volumeLabel->setText(QString("%1%").arg(val));
    if (m_engine) {
        m_engine->setVolume(static_cast<double>(val));
    }
}

void AuraControls::onMuteBtnClicked() {
    emit userInteracted();
    if (m_engine) {
        m_engine->toggleMute();
    }
}

void AuraControls::onSpeedSelected(int index) {
    emit userInteracted();
    if (m_engine && index >= 0) {
        double spd = m_speedCombo->itemData(index).toDouble();
        m_engine->setSpeed(spd);
    }
}

void AuraControls::updateTrackMenus() {
    if (!m_engine) return;
    auto tracks = m_engine->getTracks();

    // Audio tracks
    m_audioCombo->blockSignals(true);
    m_audioCombo->clear();
    m_audioCombo->addItem("Audio: Auto", -1);
    for (const auto &track : tracks) {
        if (track.type == "audio") {
            QString label = QString("#%1 %2 [%3]").arg(track.id)
                .arg(track.title.isEmpty() ? (track.language.isEmpty() ? "Audio" : track.language) : track.title)
                .arg(track.codec.isEmpty() ? "Unknown" : track.codec);
            m_audioCombo->addItem(label, track.id);
            if (track.isSelected) {
                m_audioCombo->setCurrentIndex(m_audioCombo->count() - 1);
            }
        }
    }
    m_audioCombo->blockSignals(false);

    // Subtitle tracks
    m_subCombo->blockSignals(true);
    m_subCombo->clear();
    m_subCombo->addItem("Subs: None", -1);
    for (const auto &track : tracks) {
        if (track.type == "sub") {
            QString label = QString("#%1 %2 [%3]").arg(track.id)
                .arg(track.title.isEmpty() ? (track.language.isEmpty() ? "Sub" : track.language) : track.title)
                .arg(track.codec.isEmpty() ? "" : track.codec);
            m_subCombo->addItem(label, track.id);
            if (track.isSelected) {
                m_subCombo->setCurrentIndex(m_subCombo->count() - 1);
            }
        }
    }
    m_subCombo->blockSignals(false);
}

void AuraControls::onAudioTrackSelected(int index) {
    emit userInteracted();
    if (m_engine && index >= 0) {
        int trackId = m_audioCombo->itemData(index).toInt();
        m_engine->setAudioTrack(trackId);
    }
}

void AuraControls::onSubtitleTrackSelected(int index) {
    emit userInteracted();
    if (m_engine && index >= 0) {
        int trackId = m_subCombo->itemData(index).toInt();
        m_engine->setSubtitleTrack(trackId);
    }
}

void AuraControls::onAspectRatioSelected(int index) {
    emit userInteracted();
    if (m_engine && index >= 0) {
        QString ratio = m_aspectCombo->itemData(index).toString();
        m_engine->setAspectRatio(ratio);
    }
}

bool AuraControls::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_timeLabel && event->type() == QEvent::MouseButtonPress) {
        onTimeLabelClicked();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void AuraControls::onTimeLabelClicked() {
    emit userInteracted();
    m_showRemaining = !m_showRemaining;
    setPosition(m_currentPosition);
}

QString AuraControls::formatTime(double seconds) {
    if (seconds < 0.0) seconds = 0.0;
    int totalSecs = static_cast<int>(seconds);
    int hrs = totalSecs / 3600;
    int mins = (totalSecs % 3600) / 60;
    int secs = totalSecs % 60;

    if (hrs > 0) {
        return QString("%1:%2:%3")
            .arg(hrs, 2, 10, QChar('0'))
            .arg(mins, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    }
    return QString("%1:%2")
        .arg(mins, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}
