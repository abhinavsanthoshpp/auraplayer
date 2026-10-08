#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QApplication>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("AuraPlayer");
    resize(1140, 720);
    setMinimumSize(640, 420);

    m_engine = new AuraEngine(this);
    m_engine->initialize();

    setupFramelessCanvas();
    setupTopAuraCapsule();
    applyNebulaTheme();

    m_autoHideTimer.setInterval(2400);
    connect(&m_autoHideTimer, &QTimer::timeout, this, &MainWindow::onAutoHideTimeout);

    m_osdTimer.setSingleShot(true);
    connect(&m_osdTimer, &QTimer::timeout, this, [this]() {
        m_osdLabel->hide();
    });

    connect(m_engine, &AuraEngine::playbackStarted, this, [this]() {
        m_autoHideTimer.start();
        QString fileName = QFileInfo(m_engine->currentFilePath()).fileName();
        m_capsuleTitle->setText(fileName.isEmpty() ? "AuraPlayer" : fileName);
        setWindowTitle(QString("AuraPlayer — %1").arg(fileName));
    });

    connect(m_engine, &AuraEngine::playbackStopped, this, [this]() {
        m_autoHideTimer.stop();
        m_cyberDeck->show();
        m_topCapsule->show();
        m_capsuleTitle->setText("⚡ AuraPlayer");
        setWindowTitle("AuraPlayer");
    });
}

MainWindow::~MainWindow() {
}

void MainWindow::setupFramelessCanvas() {
    auto *centralContainer = new QWidget(this);
    auto *containerLayout = new QHBoxLayout(centralContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Video Canvas occupies main body
    m_videoWidget = new AuraVideoWidget(m_engine, centralContainer);
    containerLayout->addWidget(m_videoWidget, 1);

    // Studio Drawer slides out on the right
    m_studioDrawer = new AuraStudioDrawer(m_engine, centralContainer);
    m_studioDrawer->hide();
    containerLayout->addWidget(m_studioDrawer, 0);

    setCentralWidget(centralContainer);

    // Floating Cyber Deck (Bottom Dock)
    m_cyberDeck = new AuraControls(m_engine, m_videoWidget);

    // Dynamic OSD Pill
    m_osdLabel = new QLabel(m_videoWidget);
    m_osdLabel->setObjectName("AuraDynamicOsd");
    m_osdLabel->setAlignment(Qt::AlignCenter);
    m_osdLabel->hide();

    // Wiring video widget events
    connect(m_videoWidget, &AuraVideoWidget::doubleClicked, this, &MainWindow::onToggleFullscreen);
    connect(m_videoWidget, &AuraVideoWidget::singleClicked, this, &MainWindow::onTogglePlayPause);
    connect(m_videoWidget, &AuraVideoWidget::userActivity, this, &MainWindow::onUserActivity);
    connect(m_videoWidget, &AuraVideoWidget::fileDropped, this, &MainWindow::openMedia);
    connect(m_videoWidget, &AuraVideoWidget::wheelScrolled, this, [this](int delta) {
        onVolumeDelta(delta > 0 ? 5.0 : -5.0);
    });

    // Wiring Cyber Deck events
    connect(m_cyberDeck, &AuraControls::playPauseClicked, this, &MainWindow::onTogglePlayPause);
    connect(m_cyberDeck, &AuraControls::prevClicked, this, &MainWindow::onPrevious);
    connect(m_cyberDeck, &AuraControls::nextClicked, this, &MainWindow::onNext);
    connect(m_cyberDeck, &AuraControls::stepBackClicked, this, &MainWindow::onStepBack);
    connect(m_cyberDeck, &AuraControls::stepForwardClicked, this, &MainWindow::onStepForward);
    connect(m_cyberDeck, &AuraControls::fullscreenClicked, this, &MainWindow::onToggleFullscreen);
    connect(m_cyberDeck, &AuraControls::studioToggleClicked, this, &MainWindow::onToggleStudioDrawer);
    connect(m_cyberDeck, &AuraControls::pipToggleClicked, this, &MainWindow::onToggleAlwaysOnTop);
    connect(m_cyberDeck, &AuraControls::userInteracted, this, &MainWindow::onUserActivity);

    // Wiring Studio Drawer
    connect(m_studioDrawer, &AuraStudioDrawer::trackSelected, this, &MainWindow::openMedia);
    connect(m_studioDrawer, &AuraStudioDrawer::closeRequested, this, &MainWindow::onToggleStudioDrawer);
}

void MainWindow::setupTopAuraCapsule() {
    m_topCapsule = new QWidget(m_videoWidget);
    m_topCapsule->setObjectName("AuraTopCapsule");

    auto *capsuleLayout = new QHBoxLayout(m_topCapsule);
    capsuleLayout->setContentsMargins(12, 6, 12, 6);
    capsuleLayout->setSpacing(8);

    m_capsuleTitle = new QLabel("⚡ Aura<strong>Player</strong>", m_topCapsule);
    m_capsuleTitle->setObjectName("AuraCapsuleBrand");
    m_capsuleTitle->setTextFormat(Qt::RichText);
    capsuleLayout->addWidget(m_capsuleTitle);

    capsuleLayout->addSpacing(8);

    m_capsuleOpenBtn = new QPushButton("📁 Open", m_topCapsule);
    m_capsuleOpenBtn->setObjectName("AuraCapsuleBtn");
    m_capsuleOpenBtn->setToolTip("Open Media File (Ctrl+O)");
    connect(m_capsuleOpenBtn, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    capsuleLayout->addWidget(m_capsuleOpenBtn);

    m_capsuleStreamBtn = new QPushButton("🌐 Stream", m_topCapsule);
    m_capsuleStreamBtn->setObjectName("AuraCapsuleBtn");
    m_capsuleStreamBtn->setToolTip("Play Stream URL (Ctrl+U)");
    connect(m_capsuleStreamBtn, &QPushButton::clicked, this, &MainWindow::onOpenNetworkStream);
    capsuleLayout->addWidget(m_capsuleStreamBtn);

    m_capsuleStudioBtn = new QPushButton("⚡ Studio", m_topCapsule);
    m_capsuleStudioBtn->setObjectName("AuraCapsuleBtn");
    m_capsuleStudioBtn->setToolTip("Toggle Studio Panel (L / Tab)");
    connect(m_capsuleStudioBtn, &QPushButton::clicked, this, &MainWindow::onToggleStudioDrawer);
    capsuleLayout->addWidget(m_capsuleStudioBtn);

    capsuleLayout->addStretch(1);

    m_capsulePipBtn = new QPushButton("📌", m_topCapsule);
    m_capsulePipBtn->setObjectName("AuraCapsuleIconBtn");
    m_capsulePipBtn->setToolTip("Always on Top");
    connect(m_capsulePipBtn, &QPushButton::clicked, this, &MainWindow::onToggleAlwaysOnTop);
    capsuleLayout->addWidget(m_capsulePipBtn);

    m_capsuleMaxBtn = new QPushButton("⛶", m_topCapsule);
    m_capsuleMaxBtn->setObjectName("AuraCapsuleIconBtn");
    m_capsuleMaxBtn->setToolTip("Toggle Fullscreen (F11 / F)");
    connect(m_capsuleMaxBtn, &QPushButton::clicked, this, &MainWindow::onToggleFullscreen);
    capsuleLayout->addWidget(m_capsuleMaxBtn);

    m_capsuleCloseBtn = new QPushButton("✕", m_topCapsule);
    m_capsuleCloseBtn->setObjectName("AuraCapsuleCloseBtn");
    m_capsuleCloseBtn->setToolTip("Quit AuraPlayer (Ctrl+Q)");
    connect(m_capsuleCloseBtn, &QPushButton::clicked, this, &QWidget::close);
    capsuleLayout->addWidget(m_capsuleCloseBtn);
}

void MainWindow::applyNebulaTheme() {
    QFile file(":/style.qss");
    if (!file.exists()) {
        file.setFileName("resources/style.qss");
    }
    if (file.open(QFile::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
    }
}

void MainWindow::repositionFloatingOverlays() {
    if (!m_videoWidget) return;

    int canvasW = m_videoWidget->width();
    int canvasH = m_videoWidget->height();

    // 1. Position Top Aura Capsule: centered or spanning top margin
    int capW = std::min(canvasW - 32, 780);
    int capH = 46;
    int capX = (canvasW - capW) / 2;
    int capY = 16;
    m_topCapsule->setGeometry(capX, capY, capW, capH);

    // 2. Position Floating Cyber Deck: centered near bottom
    int deckW = std::min(canvasW - 36, 880);
    int deckH = 92;
    int deckX = (canvasW - deckW) / 2;
    int deckY = canvasH - deckH - 20;
    m_cyberDeck->setGeometry(deckX, deckY, deckW, deckH);

    // 3. Position OSD Label: centered top-middle
    int osdX = (canvasW - m_osdLabel->width()) / 2;
    int osdY = capY + capH + 18;
    m_osdLabel->move(std::max(10, osdX), osdY);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    repositionFloatingOverlays();
}

void MainWindow::openMedia(const QString &path) {
    if (path.isEmpty() || !m_engine) return;

    m_studioDrawer->addFile(path);
    m_engine->loadFile(path);
    m_engine->play();

    m_capsuleTitle->setText(QFileInfo(path).fileName());
    showOsdMessage(QString("Now Playing: %1").arg(QFileInfo(path).fileName()), 2200);
    m_studioDrawer->refreshTelemetry();
}

void MainWindow::onOpenFile() {
    QString file = QFileDialog::getOpenFileName(
        this, "Select Media to Play in AuraPlayer", QString(),
        "All Media (*.mkv *.mp4 *.webm *.avi *.mov *.flv *.ts *.mp3 *.flac *.opus *.ogg *.wav);;All Files (*)"
    );
    if (!file.isEmpty()) {
        openMedia(file);
    }
}

void MainWindow::onOpenNetworkStream() {
    if (!m_streamDialog) {
        m_streamDialog = new AuraStreamDialog(this);
    }
    if (m_streamDialog->exec() == QDialog::Accepted) {
        QString url = m_streamDialog->streamUrl();
        if (!url.isEmpty()) {
            openMedia(url);
        }
    }
}

void MainWindow::onTogglePlayPause() {
    if (m_engine) m_engine->togglePause();
}

void MainWindow::onStop() {
    if (m_engine) m_engine->stop();
}

void MainWindow::onNext() {
    QString nextTrack = m_studioDrawer->playNext();
    if (!nextTrack.isEmpty()) openMedia(nextTrack);
}

void MainWindow::onPrevious() {
    QString prevTrack = m_studioDrawer->playPrevious();
    if (!prevTrack.isEmpty()) openMedia(prevTrack);
}

void MainWindow::onStepForward() {
    if (m_engine) m_engine->frameStep();
}

void MainWindow::onStepBack() {
    if (m_engine) m_engine->frameBackStep();
}

void MainWindow::onSeekRelative(double deltaSecs) {
    if (m_engine) {
        m_engine->seekRelative(deltaSecs);
        showOsdMessage(QString("Seek: %1%2s").arg(deltaSecs > 0 ? "+" : "").arg(deltaSecs, 0, 'f', 0));
    }
}

void MainWindow::onVolumeDelta(double delta) {
    if (m_engine) {
        double newVol = std::clamp(m_engine->volume() + delta, 0.0, 200.0);
        m_engine->setVolume(newVol);
        showOsdMessage(QString("Volume: %1%").arg(static_cast<int>(newVol)));
    }
}

void MainWindow::onSpeedDelta(double delta) {
    if (m_engine) {
        double newSpd = std::clamp(m_engine->speed() + delta, 0.25, 4.0);
        m_engine->setSpeed(newSpd);
        showOsdMessage(QString("Speed: %1x").arg(newSpd, 0, 'f', 2));
    }
}

void MainWindow::onToggleFullscreen() {
    m_isFullscreen = !m_isFullscreen;
    if (m_isFullscreen) {
        showFullScreen();
        m_capsuleMaxBtn->setText("⤓");
        m_autoHideTimer.start();
    } else {
        showNormal();
        m_capsuleMaxBtn->setText("⛶");
        m_cyberDeck->show();
        m_topCapsule->show();
    }
    repositionFloatingOverlays();
}

void MainWindow::onToggleAlwaysOnTop() {
    m_isAlwaysOnTop = !m_isAlwaysOnTop;
    setWindowFlag(Qt::WindowStaysOnTopHint, m_isAlwaysOnTop);
    show();
    showOsdMessage(m_isAlwaysOnTop ? "Pin: Always on Top" : "Pin: Normal Window");
}

void MainWindow::onToggleStudioDrawer() {
    if (m_studioDrawer->isVisible()) {
        m_studioDrawer->hide();
    } else {
        m_studioDrawer->show();
        m_studioDrawer->refreshTelemetry();
    }
    repositionFloatingOverlays();
}

void MainWindow::onTakeScreenshot() {
    if (m_engine) {
        m_engine->takeScreenshot();
        showOsdMessage("📸 Snapshot saved to ~/Pictures");
    }
}

void MainWindow::onUserActivity() {
    if (!m_cyberDeck->isVisible()) m_cyberDeck->show();
    if (!m_topCapsule->isVisible()) m_topCapsule->show();
    m_autoHideTimer.start();
}

void MainWindow::onAutoHideTimeout() {
    if (m_engine && m_engine->currentFilePath().isEmpty()) return;
    if (m_cyberDeck->underMouse() || m_topCapsule->underMouse() || m_studioDrawer->isVisible()) return;

    m_cyberDeck->hide();
    m_topCapsule->hide();
}

void MainWindow::showOsdMessage(const QString &text, int timeoutMs) {
    m_osdLabel->setText(text);
    m_osdLabel->adjustSize();
    repositionFloatingOverlays();
    m_osdLabel->show();
    m_osdTimer.start(timeoutMs);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    onUserActivity();

    switch (event->key()) {
    case Qt::Key_Space:
        onTogglePlayPause();
        break;
    case Qt::Key_Left:
        if (event->modifiers() & Qt::ControlModifier) onSeekRelative(-30.0);
        else if (event->modifiers() & Qt::ShiftModifier) onSeekRelative(-1.0);
        else onSeekRelative(-5.0);
        break;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ControlModifier) onSeekRelative(30.0);
        else if (event->modifiers() & Qt::ShiftModifier) onSeekRelative(1.0);
        else onSeekRelative(5.0);
        break;
    case Qt::Key_Up:
        onVolumeDelta(5.0);
        break;
    case Qt::Key_Down:
        onVolumeDelta(-5.0);
        break;
    case Qt::Key_M:
        if (m_engine) m_engine->toggleMute();
        break;
    case Qt::Key_F:
    case Qt::Key_F11:
        onToggleFullscreen();
        break;
    case Qt::Key_Escape:
        if (m_isFullscreen) onToggleFullscreen();
        break;
    case Qt::Key_S:
        onTakeScreenshot();
        break;
    case Qt::Key_Tab:
    case Qt::Key_L:
        onToggleStudioDrawer();
        break;
    case Qt::Key_BracketLeft:
        onSpeedDelta(-0.1);
        break;
    case Qt::Key_BracketRight:
        onSpeedDelta(0.1);
        break;
    case Qt::Key_Backspace:
        if (m_engine) m_engine->setSpeed(1.0);
        break;
    case Qt::Key_Z:
        if (m_engine) {
            double d = m_engine->subtitleDelay() - 0.1;
            m_engine->setSubtitleDelay(d);
            showOsdMessage(QString("Sub Delay: %1 ms").arg(static_cast<int>(d * 1000)));
        }
        break;
    case Qt::Key_X:
        if (m_engine) {
            double d = m_engine->subtitleDelay() + 0.1;
            m_engine->setSubtitleDelay(d);
            showOsdMessage(QString("Sub Delay: %1 ms").arg(static_cast<int>(d * 1000)));
        }
        break;
    case Qt::Key_J:
        if (m_engine) {
            double d = m_engine->audioDelay() - 0.1;
            m_engine->setAudioDelay(d);
            showOsdMessage(QString("Audio Delay: %1 ms").arg(static_cast<int>(d * 1000)));
        }
        break;
    case Qt::Key_K:
        if (m_engine) {
            double d = m_engine->audioDelay() + 0.1;
            m_engine->setAudioDelay(d);
            showOsdMessage(QString("Audio Delay: %1 ms").arg(static_cast<int>(d * 1000)));
        }
        break;
    case Qt::Key_O:
        if (event->modifiers() & Qt::ControlModifier) onOpenFile();
        break;
    case Qt::Key_U:
        if (event->modifiers() & Qt::ControlModifier) onOpenNetworkStream();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_engine) m_engine->stop();
    QMainWindow::closeEvent(event);
}
