#include "AuraVlcToolbar.h"
#include <QToolTip>
#include <QFrame>
#include <cmath>

// ================= AuraVlcSlider =================

AuraVlcSlider::AuraVlcSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent) {
    setMouseTracking(true);
    setRange(0, 1000);
    setCursor(Qt::PointingHandCursor);
    setObjectName("AuraVlcTimeSlider");
}

double AuraVlcSlider::ratioFromX(int x) const {
    if (width() <= 0) return 0.0;
    return std::clamp(static_cast<double>(x) / static_cast<double>(width()), 0.0, 1.0);
}

void AuraVlcSlider::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        double r = ratioFromX(event->pos().x());
        setValue(static_cast<int>(r * 1000.0));
        emit seekPercent(r);
    }
    QSlider::mousePressEvent(event);
}

void AuraVlcSlider::mouseMoveEvent(QMouseEvent *event) {
    double r = ratioFromX(event->pos().x());
    emit hoverPercent(r, event->globalPosition().toPoint());
    QSlider::mouseMoveEvent(event);
}

void AuraVlcSlider::leaveEvent(QEvent *event) {
    QToolTip::hideText();
    QSlider::leaveEvent(event);
}

// ================= AuraVlcToolbar =================

AuraVlcToolbar::AuraVlcToolbar(AuraEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine) {
    setupUi();

    if (m_engine) {
        connect(m_engine, &AuraEngine::positionChanged, this, &AuraVlcToolbar::setPosition);
        connect(m_engine, &AuraEngine::durationChanged, this, &AuraVlcToolbar::setDuration);
        connect(m_engine, &AuraEngine::playbackPaused, this, &AuraVlcToolbar::setPaused);
        connect(m_engine, &AuraEngine::volumeChanged, this, &AuraVlcToolbar::setVolume);
        connect(m_engine, &AuraEngine::muteChanged, this, &AuraVlcToolbar::setMuted);
    }
}

void AuraVlcToolbar::setupUi() {
    setObjectName("AuraVlcToolbar");
    setAttribute(Qt::WA_StyledBackground, true);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(8, 4, 8, 6);
    rootLayout->setSpacing(4);

    // 1. Advanced Controls Bar (Hidden by default, toggleable via View menu)
    m_advancedBar = new QWidget(this);
    m_advancedBar->setObjectName("AuraVlcAdvancedBar");
    auto *advLayout = new QHBoxLayout(m_advancedBar);
    advLayout->setContentsMargins(0, 0, 0, 0);
    advLayout->setSpacing(6);

    m_recordBtn = new QPushButton("🔴 Record", m_advancedBar);
    m_recordBtn->setToolTip("Record current playback stream");
    m_recordBtn->setFixedHeight(26);

    m_snapshotBtn = new QPushButton("📷 Snapshot", m_advancedBar);
    m_snapshotBtn->setToolTip("Take video frame snapshot (Shift+S)");
    m_snapshotBtn->setFixedHeight(26);

    m_abLoopBtn = new QPushButton("🔁 Loop A-B", m_advancedBar);
    m_abLoopBtn->setToolTip("Loop continuously between point A and point B");
    m_abLoopBtn->setFixedHeight(26);

    m_frameStepBtn = new QPushButton("⏭ Frame", m_advancedBar);
    m_frameStepBtn->setToolTip("Step forward frame-by-frame (E)");
    m_frameStepBtn->setFixedHeight(26);

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
    m_elapsedLabel->setObjectName("AuraVlcElapsedLabel");

    m_timeSlider = new AuraVlcSlider(Qt::Horizontal, this);

    m_durationLabel = new QLabel("00:00:00", this);
    m_durationLabel->setObjectName("AuraVlcDurationLabel");
    m_durationLabel->setCursor(Qt::PointingHandCursor);
    m_durationLabel->setToolTip("Click to toggle remaining time countdown");

    timeRow->addWidget(m_elapsedLabel);
    timeRow->addWidget(m_timeSlider, 1);
    timeRow->addWidget(m_durationLabel);
    rootLayout->addLayout(timeRow);

    // 3. Main Transport & Controls Row
    auto *controlRow = new QHBoxLayout();
    controlRow->setContentsMargins(0, 0, 0, 0);
    controlRow->setSpacing(4);

    m_playPauseBtn = new QPushButton("▶", this);
    m_playPauseBtn->setObjectName("AuraVlcPlayPauseBtn");
    m_playPauseBtn->setFixedSize(32, 30);
    m_playPauseBtn->setToolTip("Play/Pause (Space)");

    m_prevBtn = new QPushButton("⏮", this);
    m_prevBtn->setObjectName("AuraVlcToolBtn");
    m_prevBtn->setFixedSize(28, 28);
    m_prevBtn->setToolTip("Previous track in playlist (P)");

    m_stopBtn = new QPushButton("⏹", this);
    m_stopBtn->setObjectName("AuraVlcToolBtn");
    m_stopBtn->setFixedSize(28, 28);
    m_stopBtn->setToolTip("Stop playback (S)");

    m_nextBtn = new QPushButton("⏭", this);
    m_nextBtn->setObjectName("AuraVlcToolBtn");
    m_nextBtn->setFixedSize(28, 28);
    m_nextBtn->setToolTip("Next track in playlist (N)");

    m_fullscreenBtn = new QPushButton("⛶", this);
    m_fullscreenBtn->setObjectName("AuraVlcToolBtn");
    m_fullscreenBtn->setFixedSize(28, 28);
    m_fullscreenBtn->setToolTip("Toggle Fullscreen (F11 / F)");

    m_effectsBtn = new QPushButton("🎛", this);
    m_effectsBtn->setObjectName("AuraVlcToolBtn");
    m_effectsBtn->setFixedSize(28, 28);
    m_effectsBtn->setToolTip("Show Extended Settings: Equalizer, Video FX, and Audio/Sub Sync (Ctrl+E)");

    m_playlistBtn = new QPushButton("📑", this);
    m_playlistBtn->setObjectName("AuraVlcToolBtn");
    m_playlistBtn->setFixedSize(28, 28);
    m_playlistBtn->setToolTip("Toggle Playlist View (Ctrl+L)");

    m_loopBtn = new QPushButton("➡️", this);
    m_loopBtn->setObjectName("AuraVlcToolBtn");
    m_loopBtn->setFixedSize(28, 28);
    m_loopBtn->setToolTip("Loop Mode: Normal (Click to toggle Repeat All / Repeat One)");

    m_shuffleBtn = new QPushButton("🔀", this);
    m_shuffleBtn->setObjectName("AuraVlcToolBtn");
    m_shuffleBtn->setFixedSize(28, 28);
    m_shuffleBtn->setToolTip("Random / Shuffle Playlist Mode");

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
    m_muteBtn->setObjectName("AuraVlcMuteBtn");
    m_muteBtn->setFixedSize(26, 26);
    m_muteBtn->setToolTip("Mute / Unmute audio (M)");

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setObjectName("AuraVlcVolSlider");
    m_volumeSlider->setRange(0, 200);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setFixedWidth(90);
    m_volumeSlider->setToolTip("Volume (0% - 200% with VLC audio booster)");

    m_volumeLabel = new QLabel("100%", this);
    m_volumeLabel->setObjectName("AuraVlcVolLabel");
    m_volumeLabel->setFixedWidth(42);

    controlRow->addWidget(m_muteBtn);
    controlRow->addWidget(m_volumeSlider);
    controlRow->addWidget(m_volumeLabel);

    rootLayout->addLayout(controlRow);

    // Signal connections
    connect(m_timeSlider, &AuraVlcSlider::seekPercent, this, &AuraVlcToolbar::onSeekRequested);
    connect(m_timeSlider, &AuraVlcSlider::hoverPercent, this, [this](double pct, const QPoint &pos) {
        if (m_duration > 0.0) {
            double hoverSecs = pct * m_duration;
            QToolTip::showText(pos, formatVlcTime(hoverSecs), m_timeSlider);
        }
    });

    connect(m_playPauseBtn, &QPushButton::clicked, this, &AuraVlcToolbar::playPauseClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &AuraVlcToolbar::stopClicked);
    connect(m_prevBtn, &QPushButton::clicked, this, &AuraVlcToolbar::prevClicked);
    connect(m_nextBtn, &QPushButton::clicked, this, &AuraVlcToolbar::nextClicked);
    connect(m_fullscreenBtn, &QPushButton::clicked, this, &AuraVlcToolbar::fullscreenClicked);
    connect(m_effectsBtn, &QPushButton::clicked, this, &AuraVlcToolbar::extendedSettingsClicked);
    connect(m_playlistBtn, &QPushButton::clicked, this, &AuraVlcToolbar::playlistClicked);
    connect(m_loopBtn, &QPushButton::clicked, this, &AuraVlcToolbar::onLoopClicked);
    connect(m_shuffleBtn, &QPushButton::clicked, this, &AuraVlcToolbar::shuffleClicked);

    connect(m_muteBtn, &QPushButton::clicked, this, &AuraVlcToolbar::onMuteClicked);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &AuraVlcToolbar::onVolumeChanged);

    connect(m_recordBtn, &QPushButton::clicked, this, &AuraVlcToolbar::recordClicked);
    connect(m_snapshotBtn, &QPushButton::clicked, this, &AuraVlcToolbar::snapshotClicked);
    connect(m_frameStepBtn, &QPushButton::clicked, this, &AuraVlcToolbar::frameStepClicked);

    m_durationLabel->installEventFilter(this);
}

void AuraVlcToolbar::setPaused(bool paused) {
    m_playPauseBtn->setText(paused ? "▶" : "⏸");
}

void AuraVlcToolbar::setPosition(double seconds) {
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

void AuraVlcToolbar::setDuration(double seconds) {
    m_duration = seconds;
    setPosition(m_position);
}

void AuraVlcToolbar::setVolume(double volume) {
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

void AuraVlcToolbar::setMuted(bool muted) {
    m_muteBtn->setText(muted ? "🔇" : "🔊");
}

void AuraVlcToolbar::setSpeed(double speed) {
    Q_UNUSED(speed);
}

void AuraVlcToolbar::setAdvancedControlsVisible(bool visible) {
    m_advancedBar->setVisible(visible);
}

bool AuraVlcToolbar::isAdvancedControlsVisible() const {
    return m_advancedBar->isVisible();
}

bool AuraVlcToolbar::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_durationLabel && event->type() == QEvent::MouseButtonPress) {
        onDurationLabelClicked();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void AuraVlcToolbar::onSeekRequested(double percent) {
    if (m_engine && m_duration > 0.0) {
        m_engine->seek(percent * m_duration);
    }
}

void AuraVlcToolbar::onVolumeChanged(int val) {
    m_volumeLabel->setText(QString("%1%").arg(val));
    if (m_engine) {
        m_engine->setVolume(static_cast<double>(val));
    }
}

void AuraVlcToolbar::onMuteClicked() {
    if (m_engine) {
        m_engine->toggleMute();
    }
}

void AuraVlcToolbar::onLoopClicked() {
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

void AuraVlcToolbar::onDurationLabelClicked() {
    m_showRemaining = !m_showRemaining;
    setPosition(m_position);
}

QString AuraVlcToolbar::formatVlcTime(double seconds) {
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
